/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 * 
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <Source/Components/SimplePlayerCameraComponent.h>
#include <AzCore/Component/TransformBus.h>
#include <AzFramework/Components/CameraBus.h>

namespace MultiplayerSample
{
    AZ_CVAR(AZ::Vector3, cl_cameraOffset, AZ::Vector3(0.0f, -5.0f, 3.0f), nullptr, AZ::ConsoleFunctorFlags::Null, "Offset to use for the player camera");
    AZ_CVAR(float, cl_cameraBlendSpeed, 0.25f, nullptr, AZ::ConsoleFunctorFlags::Null, "Rate to blend camera to latest transform");

    SimplePlayerCameraComponentController::SimplePlayerCameraComponentController(SimplePlayerCameraComponent& parent)
        : SimplePlayerCameraComponentControllerBase(parent)
    {
        ;
    }

    void SimplePlayerCameraComponentController::OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        if (IsAutonomous())
        {
            AZ::EntityId activeCameraId;
            Camera::CameraSystemRequestBus::BroadcastResult(activeCameraId, &Camera::CameraSystemRequestBus::Events::GetActiveCamera);
            m_activeCameraEntity = AZ::Interface<AZ::ComponentApplicationRequests>::Get()->FindEntity(activeCameraId);

            AZ::TickBus::Handler::BusConnect();
        }
    }

    void SimplePlayerCameraComponentController::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        if (IsAutonomous())
        {
            AZ::TickBus::Handler::BusDisconnect();
        }
    }

    float SimplePlayerCameraComponentController::GetCameraYaw() const
    {
        return GetAimAngles().GetZ();
    }

    float SimplePlayerCameraComponentController::GetCameraPitch() const
    {
        return GetAimAngles().GetX();
    }

    float SimplePlayerCameraComponentController::GetCameraRoll() const
    {
        return GetAimAngles().GetY();
    }

    void SimplePlayerCameraComponentController::OnTick([[maybe_unused]] float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time)
    {
        if (m_activeCameraEntity != nullptr && m_activeCameraEntity->GetState() == AZ::Entity::State::Active)
        {
            const AZ::Quaternion targetRotation = AZ::Quaternion::CreateRotationZ(GetCameraYaw()) * AZ::Quaternion::CreateRotationX(GetCameraPitch());
            const AZ::Quaternion currentRotation = m_activeCameraEntity->GetTransform()->GetWorldTM().GetRotation();
            const AZ::Quaternion aimRotation = currentRotation.Slerp(targetRotation, cl_cameraBlendSpeed).GetNormalized();
            const AZ::Vector3 targetTranslation = GetEntity()->GetTransform()->GetWorldTM().GetTranslation();
            const AZ::Vector3 cameraOffset = aimRotation.TransformVector(cl_cameraOffset);
            const AZ::Transform cameraTransform = AZ::Transform::CreateFromQuaternionAndTranslation(aimRotation, targetTranslation + cameraOffset);
            m_activeCameraEntity->GetTransform()->SetWorldTM(cameraTransform);
        }
    }

    int SimplePlayerCameraComponentController::GetTickOrder()
    {
        return AZ::TICK_PRE_RENDER;
    }
}
