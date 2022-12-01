/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <GameplayEffectsNotificationBus.h>
#include <IAudioSystem.h>
#include <Source/AutoGen/GameplayEffectsComponent.AutoComponent.h>

namespace MultiplayerSample
{
    class GameplayEffectsComponent
        : public GameplayEffectsComponentBase
        , public Audio::AudioTriggerNotificationBus::MultiHandler
        , public LocalOnlyGameplayEffectsNotificationBus::Handler
    {
    public:
        AZ_MULTIPLAYER_COMPONENT(MultiplayerSample::GameplayEffectsComponent, s_gameplayEffectsComponentConcreteUuid, MultiplayerSample::GameplayEffectsComponentBase);

        static void Reflect(AZ::ReflectContext* context);
        
        void OnActivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
        void OnDeactivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;

        void HandleRPC_OnEffect(AzNetworking::IConnection* invokingConnection, const SoundEffect& effect) override;
        void HandleRPC_OnPositionalEffect(AzNetworking::IConnection* invokingConnection, const SoundEffect& effect, const AZ::Vector3& soundLocation) override;
        
        // AudioTriggerNotificationBus overrides ...
        void ReportTriggerFinished(Audio::TAudioControlID triggerId) override;

        // LocalOnlyGameplayEffectsNotificationBus overrides ...
        void OnPositionalEffect(SoundEffect effect, const AZ::Vector3& position) override;
        void OnEffect(SoundEffect effect) override;

    private:        
        AZStd::vector<AZStd::string> m_soundTriggerNames;
        AZStd::unordered_map<AZ::EntityId, AZStd::shared_ptr<AzFramework::EntitySpawnTicket>> m_spawnedEffects;

        void SpawnEffect(SoundEffect effect, const AZ::Vector3& position);
    };

    class GameplayEffectsComponentController
        : public GameplayEffectsComponentControllerBase
        , public GameplayEffectsNotificationBus::Handler
    {
    public:
        explicit GameplayEffectsComponentController(GameplayEffectsComponent& parent);

        void OnActivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
        void OnDeactivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;

        // GameplayEffectsNotificationBus overrides ...
        void OnEffect(SoundEffect effect) override;
        void OnPositionalEffect(SoundEffect effect, const AZ::Vector3& position) override;
    };
}
