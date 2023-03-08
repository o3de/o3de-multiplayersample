/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <GameplayEffectsNotificationBus.h>
#include <MultiplayerSampleTypes.h>
#include <UiGameOverBus.h>

#include <GameState/GameStateMatchEnded.h>
#include <GameState/GameStateMatchInProgress.h>
#include <GameState/GameStatePreparingMatch.h>
#include <Source/Components/Multiplayer/MatchPlayerCoinsComponent.h>
#include <Source/Components/Multiplayer/PlayerIdentityComponent.h>
#include <Source/Components/NetworkTeleportCompatibleComponent.h>
#include <Source/Components/NetworkHealthComponent.h>
#include <Source/Components/NetworkMatchComponent.h>
#include <Source/Components/NetworkRandomComponent.h>
#include <Source/Spawners/IPlayerSpawner.h>
#include <GameState/GameStateRequestBus.h>
#include <GameState/GameStateWaitingForPlayers.h>

#include "NetworkRandomComponent.h"
#include "Multiplayer/GemSpawnerComponent.h"

#if AZ_TRAIT_CLIENT
#include <AzFramework/Input/Buses/Requests/InputSystemCursorRequestBus.h>
#include <AzFramework/Input/Devices/Mouse/InputDeviceMouse.h>
#endif

namespace MultiplayerSample
{
    void NetworkMatchComponent::Reflect(AZ::ReflectContext* context)
    {
        AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context);
        if (serializeContext)
        {
            serializeContext->Class<NetworkMatchComponent, NetworkMatchComponentBase>()
                ->Version(1);
        }
        NetworkMatchComponentBase::Reflect(context);
    }

    void NetworkMatchComponent::OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        #if AZ_TRAIT_CLIENT
            AZ::Interface<NetworkMatchComponent>::Register(this);
        #endif

        if (IsNetEntityRoleAuthority() || IsNetEntityRoleServer())
        {
            PlayerIdentityNotificationBus::Handler::BusConnect();
        }
    }

    void NetworkMatchComponent::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        PlayerIdentityNotificationBus::Handler::BusDisconnect();

        #if AZ_TRAIT_CLIENT
            AZ::Interface<NetworkMatchComponent>::Unregister(this);
        #endif
    }

    bool NetworkMatchComponent::IsPlayerActionAllowed() const
    {
        // Disable player actions between rounds (rest period)
        if (GetRoundTime() <= 0 && GetRoundRestTimeRemaining() > 0)
        {
            return false;
        }

        // Don't allow player movement if the console is open (system cursor isn't unconstrainted and visible)
        #if AZ_TRAIT_CLIENT
            AzFramework::SystemCursorState systemCursorState{ AzFramework::SystemCursorState::Unknown };
            AzFramework::InputSystemCursorRequestBus::EventResult(systemCursorState, AzFramework::InputDeviceMouse::Id,
                &AzFramework::InputSystemCursorRequests::GetSystemCursorState);
            if (systemCursorState == AzFramework::SystemCursorState::UnconstrainedAndVisible)
            {
                return false;
            }
        #endif

        return true;
    }

#if AZ_TRAIT_SERVER
    void NetworkMatchComponent::OnPlayerActivated(Multiplayer::NetEntityId playerEntity)
    {
        RPC_PlayerActivated(playerEntity);
    }

    void NetworkMatchComponent::OnPlayerDeactivated(Multiplayer::NetEntityId playerEntity)
    {
        RPC_PlayerDeactivated(playerEntity);
    }
#endif

#if AZ_TRAIT_CLIENT
    void NetworkMatchComponent::HandleRPC_EndMatch(
        [[maybe_unused]] AzNetworking::IConnection* invokingConnection, [[maybe_unused]] const MatchResultsSummary& results)
    {
        if (IsNetEntityRoleClient())
        {
            UiGameOverBus::Broadcast(&UiGameOverBus::Events::SetGameOverScreenEnabled, true);
            UiGameOverBus::Broadcast(&UiGameOverBus::Events::DisplayResults, results);

            const char* playerIdentityName = nullptr;
            PlayerIdentityRequestBus::BroadcastResult(playerIdentityName, &PlayerIdentityRequestBus::Events::GetPlayerIdentityName);
            if (playerIdentityName)
            {
                if (results.m_winningPlayerName == playerIdentityName)
                {
                    // Local player is the winner
                    LocalOnlyGameplayEffectsNotificationBus::Broadcast(
                        &LocalOnlyGameplayEffectsNotificationBus::Events::OnEffect, SoundEffect::VictoryFanfare);
                }
                else
                {
                    LocalOnlyGameplayEffectsNotificationBus::Broadcast(
                        &LocalOnlyGameplayEffectsNotificationBus::Events::OnEffect, SoundEffect::LosingFanfare);
                }
            }
        }
    }
#endif  // AZ_TRAIT_CLIENT

    // Controller methods

    NetworkMatchComponentController::NetworkMatchComponentController(NetworkMatchComponent& parent)
        : NetworkMatchComponentControllerBase(parent)
    {
    }

    void NetworkMatchComponentController::OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        GameState::GameStateRequests::AddGameStateFactoryOverrideForType<GameStateWaitingForPlayers>([this]()
            {
                return AZStd::make_shared<GameStateWaitingForPlayers>(this);
            });
        GameState::GameStateRequests::AddGameStateFactoryOverrideForType<GameStatePreparingMatch>([this]()
            {
                return AZStd::make_shared<GameStatePreparingMatch>(this);
            });
        GameState::GameStateRequests::AddGameStateFactoryOverrideForType<GameStateMatchInProgress>([this]()
            {
                return AZStd::make_shared<GameStateMatchInProgress>(this);
            });
        GameState::GameStateRequests::AddGameStateFactoryOverrideForType<GameStateMatchEnded>([this]()
            {
                return AZStd::make_shared<GameStateMatchEnded>(this);
            });

        GameState::GameStateRequests::CreateAndPushNewOverridableGameStateOfType<GameStateWaitingForPlayers>();

        PlayerMatchLifecycleBus::Handler::BusConnect();
    }

    void NetworkMatchComponentController::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        PlayerMatchLifecycleBus::Handler::BusDisconnect();

        GameState::GameStateRequestBus::Broadcast(&GameState::GameStateRequestBus::Events::PopAllGameStates);

        GameState::GameStateRequests::RemoveGameStateFactoryOverrideForType<GameStateWaitingForPlayers>();
        GameState::GameStateRequests::RemoveGameStateFactoryOverrideForType<GameStatePreparingMatch>();
        GameState::GameStateRequests::RemoveGameStateFactoryOverrideForType<GameStateMatchInProgress>();
        GameState::GameStateRequests::RemoveGameStateFactoryOverrideForType<GameStateMatchEnded>();
#if AZ_TRAIT_SERVER
        m_roundTickEvent.RemoveFromQueue();
        m_restTickEvent.RemoveFromQueue();
#endif
    }

#if AZ_TRAIT_SERVER
    void NetworkMatchComponentController::StartMatch()
    {
        SetRoundTime(RoundTimeSec{ GetRoundDuration() });
        SetRoundNumber(1);
        GetGemSpawnerComponentController()->SpawnGems();

        // Tick once a second, this way we can keep the time as an 2 byte integer instead of a float.
        m_roundTickEvent.Enqueue(AZ::TimeMs{ 1000 }, true);
    }

    void NetworkMatchComponentController::EndMatch()
    {
        //Signal event to end the match
        m_roundTickEvent.RemoveFromQueue();
        m_restTickEvent.RemoveFromQueue();

        MatchResultsSummary results;

        const AZStd::vector<PlayerCoinState>& coinStates = GetMatchPlayerCoinsComponentController()->GetParent().
            GetPlayerCoinCounts();

        int highestCoins = -1;

        AZStd::vector<PlayerState> potentialWinners;

        for (const Multiplayer::NetEntityId playerNetEntity : m_players)
        {
            PlayerState state;
            const auto playerHandle = Multiplayer::GetNetworkEntityManager()->GetEntity(playerNetEntity);
            if (playerHandle.Exists())
            {
                if (PlayerIdentityComponent* identity = playerHandle.GetEntity()->FindComponent<PlayerIdentityComponent>())
                {
                    state.m_playerName = identity->GetPlayerName();
                    RespawnPlayer(playerNetEntity, PlayerResetOptions{ true, 100 });
                }
                if (const NetworkHealthComponent* armor = playerHandle.GetEntity()->FindComponent<NetworkHealthComponent>())
                {
                    // Treating health as armor
                    state.m_remainingArmor = aznumeric_cast<uint8_t>(armor->GetHealth());
                }
            }
            else
            {
                continue;
            }

            const auto coinStateIterator = AZStd::find_if(coinStates.begin(), coinStates.end(), [playerNetEntity](const PlayerCoinState& state)
                {
                    return state.m_playerId == playerNetEntity;
                });
            if (coinStateIterator != coinStates.end())
            {
                state.m_score = coinStateIterator->m_coins;
                if (highestCoins < aznumeric_cast<int>(state.m_score))
                {
                    highestCoins = aznumeric_cast<int>(state.m_score);

                    // There is no tie so far.
                    potentialWinners.clear();
                    potentialWinners.push_back(state);
                }
                else if (highestCoins == aznumeric_cast<int>(state.m_score))
                {
                    // A potential tie - decide based on remaining armor later.
                    potentialWinners.push_back(state);                    
                }
            }

            results.m_playerStates.push_back(state);
        }

        FindWinner(results, potentialWinners);

        RPC_EndMatch(results);
        GetMatchPlayerCoinsComponentController()->ResetAllCoins();
    }
#endif

    void NetworkMatchComponentController::FindWinner(MatchResultsSummary& results,
        const AZStd::vector<PlayerState>& potentialWinners)
    {
        if (potentialWinners.empty())
        {
            results.m_winningPlayerName = "No players in the match";
        }
        else if (potentialWinners.size() == 1)
        {
            results.m_winningPlayerName = potentialWinners.front().m_playerName;
        }
        else if (potentialWinners.size() > 1)
        {
            // A tie - find the player with the largest armor remaining.
            AZStd::vector<const PlayerState*> playersTiedByArmor;
            playersTiedByArmor.push_back(&potentialWinners.front());

            for (const PlayerState& potential : potentialWinners)
            {
                if (potential.m_remainingArmor == playersTiedByArmor.front()->m_remainingArmor)
                {
                    playersTiedByArmor.push_back(&potential);
                }
                else if (potential.m_remainingArmor > playersTiedByArmor.front()->m_remainingArmor)
                {
                    playersTiedByArmor.clear();
                    playersTiedByArmor.push_back(&potential);
                }
            }

            if (playersTiedByArmor.size() > 1)
            {
                // If multiple players are still tied on armor, randomly choose a player
                const AZ::u64 randomlyChosenWinnerIndex = GetNetworkRandomComponentController()->GetRandomUint64() % playersTiedByArmor.size();
                results.m_winningPlayerName = playersTiedByArmor[randomlyChosenWinnerIndex]->m_playerName;
            }
            else
            {
                results.m_winningPlayerName = playersTiedByArmor.front()->m_playerName;
            }
        }
    }

#if AZ_TRAIT_SERVER
    void NetworkMatchComponentController::StartRound()
    {
        uint16_t roundNumber = GetRoundNumber() + 1;

        // We need to do this whether or not we're going beyond the number of total rounds so that
        // the game state code can detect that it's time to end the game.
        SetRoundNumber(roundNumber);

        if (roundNumber <= GetTotalRounds())
        {
            // stop the rest timer
            m_restTickEvent.RemoveFromQueue();

            // start the round timer
            SetRoundTime(RoundTimeSec{ GetRoundDuration() });
            m_roundTickEvent.Enqueue(AZ::TimeMs{ 1000 }, true); // Tick once a second, this way we can keep the time as an 2 byte integer instead of a float.
            GetGemSpawnerComponentController()->SpawnGems();
        }
    }

    void NetworkMatchComponentController::EndRound()
    {
        // Check if we're in-between rounds, or if this is the end of the match...
        if (GetRoundNumber() < GetTotalRounds()) // In-between
        {
            // stop the round timer
            m_roundTickEvent.RemoveFromQueue();

            // start the rest timer
            ModifyRoundRestTimeRemaining() = RoundTimeSec{ GetRestDurationBetweenRounds() };
            m_restTickEvent.Enqueue(AZ::TimeMs{ 1000 }, true);

            // Respawn players before the new round starts
            for (const Multiplayer::NetEntityId playerNetEntity : m_players)
            {
                const Multiplayer::ConstNetworkEntityHandle playerHandle = Multiplayer::GetNetworkEntityManager()->GetEntity(playerNetEntity);
                if (!playerHandle.Exists())
                {
                    continue;
                }

                constexpr bool resetShields = true;
                constexpr uint16_t coinPenalty = 0;
                RespawnPlayer(playerNetEntity, PlayerResetOptions{ resetShields, coinPenalty });
            }
        }
        else // Match ended
        {
            // Incrementing the round number will trigger GameStateMatchEnded
            ModifyRoundNumber()++;
        }
    }

    void NetworkMatchComponentController::HandleRPC_PlayerActivated([[maybe_unused]] AzNetworking::IConnection* invokingConnection,
        const Multiplayer::NetEntityId& playerEntity)
    {
        const auto playerIterator = AZStd::find(m_players.begin(), m_players.end(), playerEntity);
        if (playerIterator == m_players.end())
        {
            m_players.push_back(playerEntity);
            AssignPlayerIdentity(playerEntity);
        }
    }

    void NetworkMatchComponentController::HandleRPC_PlayerDeactivated([[maybe_unused]] AzNetworking::IConnection* invokingConnection,
        const Multiplayer::NetEntityId& playerEntity)
    {
        const auto playerIterator = AZStd::find(m_players.begin(), m_players.end(), playerEntity);
        if (playerIterator != m_players.end())
        {
            m_players.erase(playerIterator);
        }
        else
        {
            AZ_Warning("NetworkMatchComponentController", false, "An unknown player deactivated %llu", aznumeric_cast<AZ::u64>(playerEntity));
        }
    }
#endif

    void NetworkMatchComponentController::OnPlayerArmorZero([[maybe_unused]] Multiplayer::NetEntityId playerEntity)
    {
#if AZ_TRAIT_SERVER
        const auto playerIterator = AZStd::find(m_players.begin(), m_players.end(), playerEntity);
        if (playerIterator != m_players.end())
        {
            if (Multiplayer::ConstNetworkEntityHandle playerHandle = Multiplayer::GetNetworkEntityManager()->GetEntity(playerEntity))
            {
                RespawnPlayer(playerEntity, PlayerResetOptions{ true, GetRespawnPenaltyPercent() });
            }
        }
        else
        {
            AZ_Warning("NetworkMatchComponentController", false, "An unknown player reported depleted armor: %llu", aznumeric_cast<AZ::u64>(playerEntity));
        }
#endif   
    }

#if AZ_TRAIT_SERVER
    void NetworkMatchComponentController::RoundTickOnceASecond()
    {
        // m_roundTickEvent is configured to tick once a second
        SetRoundTime(RoundTimeSec(GetRoundTime() - 1.f));

        if (GetRoundTime() <= RoundTimeSec(0.f))
        {
            EndRound();
        }
    }

    void NetworkMatchComponentController::RestTickOnceASecond()
    {
        // m_restTickEvent is configured to tick once a second
        SetRoundRestTimeRemaining(RoundTimeSec(GetRoundRestTimeRemaining() - 1.f));

        if (GetRoundRestTimeRemaining() <= RoundTimeSec(0.f))
        {
            StartRound();
        }
    }
#endif //AZ_TRAIT_SERVER

    void NetworkMatchComponentController::AssignPlayerIdentity(Multiplayer::NetEntityId playerEntity)
    {
        const Multiplayer::ConstNetworkEntityHandle entityHandle = Multiplayer::GetNetworkEntityManager()->GetEntity(playerEntity);
        if (entityHandle.Exists())
        {
            if (PlayerIdentityComponent* identity = entityHandle.GetEntity()->FindComponent<PlayerIdentityComponent>())
            {
                identity->AssignPlayerName(PlayerNameString::format("Player %d", m_nextPlayerId));
            }
            else
            {
                AZ_Warning("NetworkMatchComponentController", false, "Player entity did not have PlayerIdentityComponent");
            }
        }

        m_nextPlayerId++;
    }

#if AZ_TRAIT_SERVER
    void NetworkMatchComponentController::RespawnPlayer(Multiplayer::NetEntityId playerEntity, PlayerResetOptions resets)
    {
        const auto playerHandle = Multiplayer::GetNetworkEntityManager()->GetEntity(playerEntity);
        if (playerHandle.Exists())
        {
            // reset state
            if (PlayerIdentityComponent* identity = playerHandle.GetEntity()->FindComponent<PlayerIdentityComponent>())
            {
                identity->RPC_ResetPlayerState(resets);
            }

            // move to valid respawn point
            if (NetworkTeleportCompatibleComponent* teleport = playerHandle.GetEntity()->FindComponent<NetworkTeleportCompatibleComponent>())
            {
                AZStd::pair<Multiplayer::PrefabEntityId, AZ::Transform> entityParams = 
                    AZ::Interface<IPlayerSpawner>::Get()->GetNextPlayerSpawn();

                teleport->Teleport(entityParams.second.GetTranslation());
            }
        }
        else
        {
            AZ_Warning("NetworkMatchComponentController", false, "Attempted respawn of an unknown player: %llu", aznumeric_cast<AZ::u64>(playerEntity));
        }
    }
#endif
}
