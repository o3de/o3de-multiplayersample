/*
 * All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
 * its licensors.
 *
 * For complete copyright and license terms please see the LICENSE at the root of this
 * distribution (the "License"). All use of this software is governed by the License,
 * or, if provided, by the license below or the license accompanying this file. Do not
 * remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *
 */

#pragma once

#include <Source/AutoGen/AnimatedHitVolumesComponent.AutoComponent.h>
#include <Multiplayer/Components/NetBindComponent.h>
#include <Integration/ActorComponentBus.h>

namespace Physics
{
    class CharacterRequests;
    class CharacterHitDetectionConfiguration;
}

namespace MultiplayerSample
{
    class AnimatedHitVolumesComponent
        : public AnimatedHitVolumesComponentBase
        , private EMotionFX::Integration::ActorComponentNotificationBus::Handler
    {
    public:
        struct AnimatedHitVolume final
        {
            AnimatedHitVolume
            (
                AzNetworking::ConnectionId connectionId,
                Physics::CharacterRequests* character,
                const char* hitVolumeName,
                const Physics::ColliderConfiguration* colliderConfig,
                const Physics::ShapeConfiguration* shapeConfig,
                const uint32_t jointIndex
            );

            ~AnimatedHitVolume() = default;

            void UpdateTransform(const AZ::Transform& transform);
            void SyncToCurrentTransform();

            Multiplayer::RewindableObject<AZ::Transform, Multiplayer::RewindHistorySize> m_transform;
            AZStd::shared_ptr<Physics::Shape> m_physicsShape;

            // Cached so we don't have to do subsequent lookups by name
            const Physics::ColliderConfiguration* m_colliderConfig = nullptr;
            const Physics::ShapeConfiguration* m_shapeConfig = nullptr;
            AZ::Transform m_colliderOffSetTransform;
            const AZ::u32 m_jointIndex = 0;
        };

        AZ_MULTIPLAYER_COMPONENT(MultiplayerSample::AnimatedHitVolumesComponent, s_animatedHitVolumesComponentConcreteUuid, MultiplayerSample::AnimatedHitVolumesComponentBase);

        static void Reflect(AZ::ReflectContext* context);

        AnimatedHitVolumesComponent();

        void OnInit() override;
        void OnActivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
        void OnDeactivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;

    private:
        void Update(AZ::TimeMs deltaTimeMs);

        void OnTransformUpdate(const AZ::Transform& transform);
        void OnSyncRewind();

        void CreateHitVolumes();
        void DestroyHitVolumes();

        //! ActorComponentNotificationBus::Handler
        //! @{
        void OnActorInstanceCreated(EMotionFX::ActorInstance* actorInstance) override;
        void OnActorInstanceDestroyed(EMotionFX::ActorInstance* actorInstance) override;
        //! @}

        Physics::CharacterRequests* m_physicsCharacter = nullptr;
        EMotionFX::Integration::ActorComponentRequests* m_actorComponent = nullptr;
        const Physics::CharacterColliderConfiguration* m_hitDetectionConfig = nullptr;

        AZStd::vector<AnimatedHitVolume> m_animatedHitVolumes;

        AZ::ScheduledEvent m_updateEvent;
        Multiplayer::EntitySyncRewindEvent::Handler m_syncRewindHandler = Multiplayer::EntitySyncRewindEvent::Handler([this]() { OnSyncRewind(); });
    };
}
