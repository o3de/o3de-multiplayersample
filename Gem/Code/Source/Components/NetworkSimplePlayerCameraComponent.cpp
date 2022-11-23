/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 * 
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AzCore/Component/ComponentApplicationBus.h>
#include <Source/Components/NetworkAiComponent.h>
#include <Source/Components/NetworkSimplePlayerCameraComponent.h>
#include <AzCore/Component/TransformBus.h>
#include <AzFramework/Components/CameraBus.h>
#include <AzFramework/Physics/PhysicsScene.h>
#include <AzFramework/Physics/Common/PhysicsSceneQueries.h>
#include <Multiplayer/IMultiplayer.h>
#include <DebugDraw/DebugDrawBus.h>

namespace MultiplayerSample
{
    AZ_CVAR(AZ::Vector3, cl_cameraOffset, AZ::Vector3(0.5f, 0.f, 1.5f), nullptr, AZ::ConsoleFunctorFlags::DontReplicate, "Offset to use for the player camera");
    AZ_CVAR(AZ::Vector3, cl_cameraColliderSize, AZ::Vector3(0.7f, 0.1f, 0.5f), nullptr, AZ::ConsoleFunctorFlags::DontReplicate, "Temporary collider size for player camera");
    AZ_CVAR(bool, cl_drawCameraCollider, false, nullptr, AZ::ConsoleFunctorFlags::DontReplicate, "Draw the camera collider");
    AZ_CVAR(bool, cl_cameraBlendingEnabled, false, nullptr, AZ::ConsoleFunctorFlags::DontReplicate, "When active, blends the camera aim angles.");


    NetworkSimplePlayerCameraComponentController::NetworkSimplePlayerCameraComponentController(NetworkSimplePlayerCameraComponent& parent)
        : NetworkSimplePlayerCameraComponentControllerBase(parent)
    {
        ;
    }

    void NetworkSimplePlayerCameraComponentController::OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        m_physicsSceneInterface = AZ::Interface<AzPhysics::SceneInterface>::Get();
        if (m_physicsSceneInterface)
        {
            m_physicsSceneHandle = m_physicsSceneInterface->GetSceneHandle(AzPhysics::DefaultPhysicsSceneName);
        }

        // Synchronize aim angles with initial transform
        AZ::Vector3& aimAngles = ModifyAimAngles();
        aimAngles.SetZ(GetEntity()->GetTransform()->GetLocalRotation().GetZ());
        SetSyncAimImmediate(true);

        if (IsNetEntityRoleAutonomous())
        {
            m_aiEnabled = FindComponent<NetworkAiComponent>()->GetEnabled();
            if (!m_aiEnabled)
            {
                AZ::EntityId activeCameraId;
                Camera::CameraSystemRequestBus::BroadcastResult(activeCameraId, &Camera::CameraSystemRequestBus::Events::GetActiveCamera);
                m_activeCameraEntity = AZ::Interface<AZ::ComponentApplicationRequests>::Get()->FindEntity(activeCameraId);
            }
        }

        AZ::TickBus::Handler::BusConnect();
    }

    void NetworkSimplePlayerCameraComponentController::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        if(AZ::TickBus::Handler::BusIsConnected())
        { 
            AZ::TickBus::Handler::BusDisconnect();
        }
    }

    float NetworkSimplePlayerCameraComponentController::GetCameraYaw() const
    {
        return GetAimAngles().GetZ();
    }

    float NetworkSimplePlayerCameraComponentController::GetCameraPitch() const
    {
        return GetAimAngles().GetX();
    }

    float NetworkSimplePlayerCameraComponentController::GetCameraRoll() const
    {
        return GetAimAngles().GetY();
    }

    float NetworkSimplePlayerCameraComponentController::GetCameraYawPrevious() const
    {
        return GetAimAnglesPrevious().GetZ();
    }

    float NetworkSimplePlayerCameraComponentController::GetCameraPitchPrevious() const
    {
        return GetAimAnglesPrevious().GetX();
    }

    float NetworkSimplePlayerCameraComponentController::GetCameraRollPrevious() const
    {
        return GetAimAnglesPrevious().GetY();
    }

    AZ::Transform NetworkSimplePlayerCameraComponentController::GetCameraTransform(bool collisionEnabled) const
    {
        // exclude roll from any rotation calculations
        const AZ::Quaternion targetYaw = AZ::Quaternion::CreateRotationZ(GetCameraYaw());
        const AZ::Quaternion targetRotation = targetYaw * AZ::Quaternion::CreateRotationX(GetCameraPitch());
        AZ::Quaternion aimRotation = targetRotation;

        if (IsNetEntityRoleAutonomous() && cl_cameraBlendingEnabled)
        {
            const float blendFactor = Multiplayer::GetMultiplayer()->GetCurrentBlendFactor();

            if (!GetSyncAimImmediate() && !AZ::IsClose(blendFactor, 1.0f))
            {
                const AZ::Quaternion prevRotation = AZ::Quaternion::CreateRotationZ(GetCameraYawPrevious()) * AZ::Quaternion::CreateRotationX(GetCameraPitchPrevious());
                if (!prevRotation.IsClose(targetRotation))
                {
                    aimRotation = prevRotation.Slerp(targetRotation, blendFactor).GetNormalized();
                }
            }
        }

        const AZ::Vector3 targetTranslation = GetEntity()->GetTransform()->GetWorldTM().GetTranslation();
        const AZ::Vector3 cameraPivotOffset = targetYaw.TransformVector(cl_cameraOffset);

        if(collisionEnabled)
        { 
            AZ::Transform cameraTransform = AZ::Transform::CreateFromQuaternionAndTranslation(aimRotation, targetTranslation + cameraPivotOffset);
            ApplySpringArm(cameraTransform);
            return cameraTransform;
        }
        else
        { 
            const AZ::Vector3 cameraOffset = cameraPivotOffset + aimRotation.TransformVector(AZ::Vector3(0.f, GetMaxFollowDistance(), 0.f));
            return AZ::Transform::CreateFromQuaternionAndTranslation(aimRotation, targetTranslation + cameraOffset);
        }
    }

    void NetworkSimplePlayerCameraComponentController::OnTick([[maybe_unused]] float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time)
    {
        if (m_activeCameraEntity != nullptr && m_activeCameraEntity->GetState() == AZ::Entity::State::Active)
        {
            const AZ::Transform& transform = GetCameraTransform(GetEnableCollision());
            m_activeCameraEntity->GetTransform()->SetWorldTM(transform);
        }

        if (GetSyncAimImmediate())
        {
            SetSyncAimImmediate(false);

            if (!m_activeCameraEntity)
            {
                // the server no longer needs to tick after the initial sync
                AZ::TickBus::Handler::BusDisconnect();
            }
        }
    }

    void NetworkSimplePlayerCameraComponentController::ApplySpringArm(AZ::Transform& inOutTransform) const
    {
        // TODO replace cl_cameraColliderSize with dynamic box based on the camera near plane world dimensions
        const AZ::EntityId ignoreEntityId = GetEntityId();
        const AZ::Vector3 cameraPivot = inOutTransform.GetTranslation();
        const AZ::Vector3 direction = -inOutTransform.GetBasisY();
        const float maxDistance = GetMaxFollowDistance();
        float distance = maxDistance;

        // trace from the target to the camera position
        auto request = AzPhysics::ShapeCastRequestHelpers::CreateBoxCastRequest(
                    cl_cameraColliderSize, inOutTransform, direction, maxDistance,
                    AzPhysics::SceneQuery::QueryType::StaticAndDynamic,
                    AzPhysics::CollisionGroup::All, 
                    [ignoreEntityId](const AzPhysics::SimulatedBody* body, [[maybe_unused]] const Physics::Shape* shape)
                    {
                        return body->GetEntityId() != ignoreEntityId ? AzPhysics::SceneQuery::QueryHitType::Block
                                                             : AzPhysics::SceneQuery::QueryHitType::None;
                    });
        AzPhysics::SceneQueryHits result = m_physicsSceneInterface->QueryScene(m_physicsSceneHandle, &request);
        if (result && !result.m_hits.empty())
        {
            // include the collision offset so we are not intersecting the surface
            distance = result.m_hits[0].m_distance - GetCollisionOffset();
            distance = AZ::GetClamp(distance, GetMinFollowDistance(), maxDistance);
        }

        inOutTransform.SetTranslation(cameraPivot + direction * distance);

        if (cl_drawCameraCollider && distance < maxDistance)
        {
            if (auto debugDraw = DebugDraw::DebugDrawRequestBus::FindFirstHandler(); debugDraw != nullptr)
            {
                const AZ::Vector3 colliderSize = cl_cameraColliderSize;
                debugDraw->DrawObb(
                    AZ::Obb::CreateFromPositionRotationAndHalfLengths(
                        inOutTransform.GetTranslation(),
                        inOutTransform.GetRotation(),
                        colliderSize * 0.5f),
                    AZ::Color(1.f,0.f,0.f,0.1f), 
                    1.f);
            }

            inOutTransform.SetTranslation(cameraPivot + direction * maxDistance);
        }
    }

    int NetworkSimplePlayerCameraComponentController::GetTickOrder()
    {
        return AZ::TICK_PRE_RENDER;
    }
}
