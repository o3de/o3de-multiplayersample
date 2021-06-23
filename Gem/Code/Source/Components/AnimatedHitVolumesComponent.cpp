/*
 * Copyright (c) Contributors to the Open 3D Engine Project
 * 
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <Source/Components/AnimatedHitVolumesComponent.h>
#include <AzFramework/Physics/Common/PhysicsTypes.h>
#include <AzFramework/Physics/CharacterBus.h>
#include <AzFramework/Physics/Character.h>
#include <AzFramework/Physics/SystemBus.h>
#include <AzFramework/Physics/Material.h>
#include <MCore/Source/AzCoreConversions.h>
#include <Integration/ActorComponentBus.h>
#include <Integration/AnimGraphNetworkingBus.h>

namespace MultiplayerSample
{
    AZ_CVAR(bool, bg_DrawArticulatedHitVolumes, false, nullptr, AZ::ConsoleFunctorFlags::Null, "Enables debug draw of articulated hit volumes");
    AZ_CVAR(float, bg_DrawDebugHitVolumeLifetime, 0.0f, nullptr, AZ::ConsoleFunctorFlags::Null, "The lifetime for hit volume draw-debug shapes");

    AZ_CVAR(float, bg_RewindPositionTolerance, 0.0001f, nullptr, AZ::ConsoleFunctorFlags::Null, "Don't sync the physx entity if the square of delta position is less than this value");
    AZ_CVAR(float, bg_RewindOrientationTolerance, 0.001f, nullptr, AZ::ConsoleFunctorFlags::Null, "Don't sync the physx entity if the square of delta orientation is less than this value");

    AnimatedHitVolumesComponent::AnimatedHitVolume::AnimatedHitVolume
    (
        AzNetworking::ConnectionId connectionId,
        Physics::CharacterRequests* character,
        const char* hitVolumeName,
        const Physics::ColliderConfiguration* colliderConfig,
        const Physics::ShapeConfiguration* shapeConfig,
        const uint32_t jointIndex
    )
        : m_colliderConfig(colliderConfig)
        , m_shapeConfig(shapeConfig)
        , m_jointIndex(jointIndex)
    {
        m_transform.SetOwningConnectionId(connectionId);

        m_colliderOffSetTransform = AZ::Transform::CreateFromQuaternionAndTranslation(m_colliderConfig->m_rotation, m_colliderConfig->m_position);

        if (m_colliderConfig->m_isExclusive)
        {
            Physics::SystemRequestBus::BroadcastResult(m_physicsShape, &Physics::SystemRequests::CreateShape, *m_colliderConfig, *m_shapeConfig);
        }
        else
        {
            Physics::ColliderConfiguration colliderConfiguration = *m_colliderConfig;
            colliderConfiguration.m_isExclusive = true;
            colliderConfiguration.m_isSimulated = false;
            colliderConfiguration.m_isInSceneQueries = true;
            Physics::SystemRequestBus::BroadcastResult(m_physicsShape, &Physics::SystemRequests::CreateShape, colliderConfiguration, *m_shapeConfig);
        }

        if (m_physicsShape)
        {
            m_physicsShape->SetName(hitVolumeName);
            m_physicsShape->SetCollisionLayer(AzPhysics::CollisionLayer::TouchBend);
            character->GetCharacter()->AttachShape(m_physicsShape);
        }
    }

    void AnimatedHitVolumesComponent::AnimatedHitVolume::UpdateTransform(const AZ::Transform& transform)
    {
        m_transform = transform;
        m_physicsShape->SetLocalPose(transform.GetTranslation(), transform.GetRotation());
    }

    void AnimatedHitVolumesComponent::AnimatedHitVolume::SyncToCurrentTransform()
    {
        const AZ::Transform& rewoundTransform = m_transform.Get();
        const AZ::Transform  physicsTransform = AZ::Transform::CreateFromQuaternionAndTranslation(m_physicsShape->GetLocalPose().second, m_physicsShape->GetLocalPose().first);

        // Don't call SetLocalPose unless the transforms are actually different
        const AZ::Vector3 positionDelta = physicsTransform.GetTranslation() - rewoundTransform.GetTranslation();
        const AZ::Quaternion orientationDelta = physicsTransform.GetRotation() - rewoundTransform.GetRotation();

        if ((positionDelta.GetLengthSq() >= bg_RewindPositionTolerance) || (orientationDelta.GetLengthSq() >= bg_RewindOrientationTolerance))
        {
            m_physicsShape->SetLocalPose(rewoundTransform.GetTranslation(), rewoundTransform.GetRotation());
        }
    }

    void AnimatedHitVolumesComponent::AnimatedHitVolumesComponent::Reflect(AZ::ReflectContext* context)
    {
        AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context);
        if (serializeContext)
        {
            serializeContext->Class<AnimatedHitVolumesComponent, AnimatedHitVolumesComponentBase>()
                ->Version(1);
        }
        AnimatedHitVolumesComponentBase::Reflect(context);
    }

    AnimatedHitVolumesComponent::AnimatedHitVolumesComponent()
        : m_syncRewindHandler([this]() { OnSyncRewind(); })
        , m_preRenderHandler([this](float deltaTime, float blendFactor) { OnPreRender(deltaTime, blendFactor); })
    {
        ;
    }

    void AnimatedHitVolumesComponent::OnInit()
    {
        ;
    }

    void AnimatedHitVolumesComponent::OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        EMotionFX::Integration::ActorComponentNotificationBus::Handler::BusConnect(GetEntityId());
        GetNetBindComponent()->AddEntitySyncRewindEventHandler(m_syncRewindHandler);
        m_physicsCharacter = Physics::CharacterRequestBus::FindFirstHandler(GetEntityId());
    }

    void AnimatedHitVolumesComponent::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        DestroyHitVolumes();
        EMotionFX::Integration::ActorComponentNotificationBus::Handler::BusDisconnect();
    }

    void AnimatedHitVolumesComponent::OnPreRender([[maybe_unused]] float deltaTime, [[maybe_unused]] float blendFactor)
    {
        if (m_animatedHitVolumes.size() <= 0)
        {
            CreateHitVolumes();
        }

        AZ::Vector3 position, scale;
        AZ::Quaternion rotation;
        for (AnimatedHitVolume& hitVolume : m_animatedHitVolumes)
        {
            m_actorComponent->GetJointTransformComponents(hitVolume.m_jointIndex, EMotionFX::Integration::Space::ModelSpace, position, rotation, scale);
            hitVolume.UpdateTransform(AZ::Transform::CreateFromQuaternionAndTranslation(rotation, position) * hitVolume.m_colliderOffSetTransform);
        }
    }

    void AnimatedHitVolumesComponent::OnTransformUpdate([[maybe_unused]] const AZ::Transform& transform)
    {
        OnSyncRewind();
    }

    void AnimatedHitVolumesComponent::OnSyncRewind()
    {
        if (m_physicsCharacter && m_physicsCharacter->GetCharacter())
        {
            uint32_t frameId = static_cast<uint32_t>(Multiplayer::GetNetworkTime()->GetHostFrameId());
            m_physicsCharacter->GetCharacter()->SetFrameId(frameId);
        }

        for (AnimatedHitVolume& hitVolume : m_animatedHitVolumes)
        {
            hitVolume.SyncToCurrentTransform();
        }
    }

    void AnimatedHitVolumesComponent::CreateHitVolumes()
    {
        if (m_physicsCharacter == nullptr || m_actorComponent == nullptr)
        {
            return;
        }

        const Physics::AnimationConfiguration* physicsConfig = m_actorComponent->GetPhysicsConfig();
        if (physicsConfig == nullptr)
        {
            return;
        }

        m_hitDetectionConfig = &physicsConfig->m_hitDetectionConfig;
        const AzNetworking::ConnectionId owningConnectionId = GetNetBindComponent()->GetOwningConnectionId();

        m_animatedHitVolumes.reserve(m_hitDetectionConfig->m_nodes.size());
        for (const Physics::CharacterColliderNodeConfiguration& nodeConfig : m_hitDetectionConfig->m_nodes)
        {
            const AZStd::size_t jointIndex = m_actorComponent->GetJointIndexByName(nodeConfig.m_name.c_str());
            if (jointIndex == EMotionFX::Integration::ActorComponentRequests::s_invalidJointIndex)
            {
                continue;
            }

            for (const AzPhysics::ShapeColliderPair& coliderPair : nodeConfig.m_shapes)
            {
                const Physics::ColliderConfiguration* colliderConfig = coliderPair.first.get();
                Physics::ShapeConfiguration* shapeConfig = coliderPair.second.get();
                m_animatedHitVolumes.emplace_back(owningConnectionId, m_physicsCharacter, nodeConfig.m_name.c_str(), colliderConfig, shapeConfig, aznumeric_cast<uint32_t>(jointIndex));
            }
        }
    }

    void AnimatedHitVolumesComponent::DestroyHitVolumes()
    {
        m_animatedHitVolumes.clear();
    }

    void AnimatedHitVolumesComponent::OnActorInstanceCreated([[maybe_unused]] EMotionFX::ActorInstance* actorInstance)
    {
        m_actorComponent = EMotionFX::Integration::ActorComponentRequestBus::FindFirstHandler(GetEntity()->GetId());
    }

    void AnimatedHitVolumesComponent::OnActorInstanceDestroyed([[maybe_unused]] EMotionFX::ActorInstance* actorInstance)
    {
        m_actorComponent = nullptr;
    }
}
