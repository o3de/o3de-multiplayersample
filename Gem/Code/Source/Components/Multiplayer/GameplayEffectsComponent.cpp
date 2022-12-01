/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AzCore/Serialization/SerializeContext.h>
#include <Components/PerfTest/NetworkPrefabSpawnerComponent.h>
#include <LmbrCentral/Audio/AudioTriggerComponentBus.h>
#include <Source/Components/Multiplayer/GameplayEffectsComponent.h>

AZ_CVAR(bool, mps_enableMissingAudioTriggerWarnings, false, nullptr, AZ::ConsoleFunctorFlags::Null,
    "Reports warnings whenever a game effect is missing an audio trigger defined in Game Playe Effects component.");

namespace MultiplayerSample
{
    void GameplayEffectsComponent::Reflect(AZ::ReflectContext* context)
    {
        AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context);
        if (serializeContext)
        {
            serializeContext->Class<GameplayEffectsComponent, GameplayEffectsComponentBase>()
                ->Version(1);
        }
        GameplayEffectsComponentBase::Reflect(context);
    }

    void GameplayEffectsComponent::OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        if (IsNetEntityRoleClient())
        {
            LocalOnlyGameplayEffectsNotificationBus::Handler::BusConnect();
        }

        m_soundTriggerNames.clear();
        m_soundTriggerNames.resize(SoundEffectNamespace::SoundEffectCount);
        
        m_soundTriggerNames[aznumeric_cast<int>(SoundEffect::PlayerFootSteps)] = GetPlayerFootSteps();
        m_soundTriggerNames[aznumeric_cast<int>(SoundEffect::PlayerExertion)] = GetPlayerExertion();
        m_soundTriggerNames[aznumeric_cast<int>(SoundEffect::PlayerKnockedDown)] = GetPlayerKnockedDown();
        m_soundTriggerNames[aznumeric_cast<int>(SoundEffect::ArmorBreaking)] = GetArmorBreaking();
        m_soundTriggerNames[aznumeric_cast<int>(SoundEffect::ArmorMend)] = GetArmorMend();
        m_soundTriggerNames[aznumeric_cast<int>(SoundEffect::PlayerOuch)] = GetPlayerOuch();
        m_soundTriggerNames[aznumeric_cast<int>(SoundEffect::LadderClimb)] = GetLadderClimb();
        m_soundTriggerNames[aznumeric_cast<int>(SoundEffect::ShutDown)] = GetShutDown();

        m_soundTriggerNames[aznumeric_cast<int>(SoundEffect::CountDown)] = GetCountDown();
        m_soundTriggerNames[aznumeric_cast<int>(SoundEffect::GemPickup)] = GetGemPickup();
        m_soundTriggerNames[aznumeric_cast<int>(SoundEffect::VictoryFanfare)] = GetVictoryFanfare();
        m_soundTriggerNames[aznumeric_cast<int>(SoundEffect::LosingFanfare)] = GetLosingFanfare();
        m_soundTriggerNames[aznumeric_cast<int>(SoundEffect::RoundStart)] = GetRoundStart();
        m_soundTriggerNames[aznumeric_cast<int>(SoundEffect::RoundEnd)] = GetRoundEnd();
        m_soundTriggerNames[aznumeric_cast<int>(SoundEffect::GameEnd)] = GetGameEnd();

        m_soundTriggerNames[aznumeric_cast<int>(SoundEffect::LaserPistolMuzzleFlash)] = GetLaserPistolMuzzleFlash();
        m_soundTriggerNames[aznumeric_cast<int>(SoundEffect::LaserPistolImpact)] = GetLaserPistolImpact();

        m_soundTriggerNames[aznumeric_cast<int>(SoundEffect::BubbleGunBuildup)] = GetBubbleGunBuildup();
        m_soundTriggerNames[aznumeric_cast<int>(SoundEffect::BubbleGunMuzzleFlash)] = GetBubbleGunMuzzleFlash();
        m_soundTriggerNames[aznumeric_cast<int>(SoundEffect::BubbleGunProjectile)] = GetBubbleGunProjectile();
        m_soundTriggerNames[aznumeric_cast<int>(SoundEffect::BubbleGunImpact)] = GetBubbleGunImpact();

        m_soundTriggerNames[aznumeric_cast<int>(SoundEffect::JumpPadLaunch)] = GetJumpPadLaunch();

        m_soundTriggerNames[aznumeric_cast<int>(SoundEffect::EnergyBallTrapRisingOutOfTheGround)] = GetEnergyBallTrapRisingOutOfTheGround();
        m_soundTriggerNames[aznumeric_cast<int>(SoundEffect::EnergyBallTrapBuildup)] = GetEnergyBallTrapBuildup();
        m_soundTriggerNames[aznumeric_cast<int>(SoundEffect::EnergyBallTrapProjectile)] = GetEnergyBallTrapProjectile();
        m_soundTriggerNames[aznumeric_cast<int>(SoundEffect::EnergyBallTrapImpact)] = GetEnergyBallTrapImpact();
        m_soundTriggerNames[aznumeric_cast<int>(SoundEffect::EnergyBallTrapOnCooldown)] = GetEnergyBallTrapOnCooldown();
    }

    void GameplayEffectsComponent::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        Audio::AudioTriggerNotificationBus::MultiHandler::BusDisconnect();
        LocalOnlyGameplayEffectsNotificationBus::Handler::BusDisconnect();
    }

    void GameplayEffectsComponent::HandleRPC_OnEffect([[maybe_unused]] AzNetworking::IConnection* invokingConnection,
        const SoundEffect& effect)
    {
        // Spawn at the local camera position
        AZ::EntityId camera;
        Camera::CameraSystemRequestBus::BroadcastResult(camera, &Camera::CameraSystemRequestBus::Events::GetActiveCamera);
        if (camera.IsValid())
        {
            AZ::Vector3 cameraPosition = AZ::Vector3::CreateZero();
            AZ::TransformBus::EventResult(cameraPosition, camera, &AZ::TransformBus::Events::GetWorldTranslation);
            SpawnEffect(effect, cameraPosition);
        }

    }

    void GameplayEffectsComponent::HandleRPC_OnPositionalEffect([[maybe_unused]] AzNetworking::IConnection* invokingConnection,
        const SoundEffect& effect, const AZ::Vector3& soundLocation)
    {
        SpawnEffect(effect, soundLocation);
    }

    void GameplayEffectsComponent::ReportTriggerFinished([[maybe_unused]] Audio::TAudioControlID triggerId)
    {
        const Audio::TriggerNotificationIdType busId = *Audio::AudioTriggerNotificationBus::GetCurrentBusId();
        const AZ::EntityId entityId = aznumeric_cast<AZ::EntityId>(busId.m_owner);
        Audio::AudioTriggerNotificationBus::MultiHandler::BusDisconnect(busId);

        // Remove the prefab that played the sound trigger
        const auto iterator = m_spawnedEffects.find(entityId);
        if (iterator != m_spawnedEffects.end())
        {
            m_spawnedEffects.erase(iterator);
        }
    }

    void GameplayEffectsComponent::OnPositionalEffect(SoundEffect effect, const AZ::Vector3& position)
    {
        HandleRPC_OnPositionalEffect(nullptr, effect, position);
    }

    void GameplayEffectsComponent::OnEffect(SoundEffect effect)
    {
        HandleRPC_OnEffect(nullptr, effect);
    }

    void GameplayEffectsComponent::SpawnEffect(SoundEffect effect, const AZ::Vector3& position)
    {
        const char* triggerName = nullptr;
        const AZStd::size_t effectId = aznumeric_cast<AZStd::size_t>(effect);
        if (effectId < m_soundTriggerNames.size())
        {
            triggerName = m_soundTriggerNames[effectId].c_str();
        }

        if (!triggerName || strlen(triggerName) == 0)
        {
            if (mps_enableMissingAudioTriggerWarnings)
            {
                const AZStd::string eventName(SoundEffectNamespace::ToString(effect));
                AZ_Warning("MultiplayerSample", false, "Audio trigger wasn't specified for effect [%s] on GameplayEffectsComponent", eventName.c_str());
            }
            return;
        }

        PrefabCallbacks callbacks;
        callbacks.m_onActivateCallback = [this, triggerName](
            AZStd::shared_ptr<AzFramework::EntitySpawnTicket>&& ticket,
            [[maybe_unused]] AzFramework::SpawnableConstEntityContainerView view)
        {            
            for (const AZ::Entity* entity : view)
            {
                LmbrCentral::AudioTriggerComponentRequests* audioTrigger =
                    LmbrCentral::AudioTriggerComponentRequestBus::FindFirstHandler(entity->GetId());
                if (audioTrigger)
                {
                    Audio::AudioTriggerNotificationBus::MultiHandler::BusConnect(Audio::TriggerNotificationIdType{ entity->GetId() });
                    m_spawnedEffects.emplace(entity->GetId(), move(ticket));
                    audioTrigger->ExecuteTrigger(triggerName);
                    break;
                }
            }
        };

        // Spawn a prefab with an audio trigger, play the effect and clean it up when it finishes.
        const AZ::Transform t = AZ::Transform::CreateFromQuaternionAndTranslation(AZ::Quaternion::CreateIdentity(), position);
        GetNetworkPrefabSpawnerComponent()->SpawnDefaultPrefab(t, callbacks);
    }


    GameplayEffectsComponentController::GameplayEffectsComponentController(GameplayEffectsComponent& parent)
        : GameplayEffectsComponentControllerBase(parent)
    {
    }

    void GameplayEffectsComponentController::OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        GameplayEffectsNotificationBus::Handler::BusConnect();
    }

    void GameplayEffectsComponentController::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        GameplayEffectsNotificationBus::Handler::BusDisconnect();
    }

    void GameplayEffectsComponentController::OnEffect(SoundEffect effect)
    {
        RPC_OnEffect(effect);
    }

    void GameplayEffectsComponentController::OnPositionalEffect(SoundEffect effect, const AZ::Vector3& position)
    {
        RPC_OnPositionalEffect(effect, position);
    }
}
