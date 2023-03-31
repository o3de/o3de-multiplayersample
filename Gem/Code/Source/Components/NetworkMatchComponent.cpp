/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AzCore/Component/TransformBus.h>
#include <AzCore/Preprocessor/EnumReflectUtils.h>

#include <GameplayEffectsNotificationBus.h>
#include <MultiplayerSampleTypes.h>
#include <UiGameOverBus.h>

#include <Source/Components/Multiplayer/GemSpawnerComponent.h>
#include <Source/Components/Multiplayer/MatchPlayerCoinsComponent.h>
#include <Source/Components/Multiplayer/PlayerIdentityComponent.h>
#include <Source/Components/NetworkTeleportCompatibleComponent.h>
#include <Source/Components/NetworkHealthComponent.h>
#include <Source/Components/NetworkMatchComponent.h>
#include <Source/Components/NetworkRandomComponent.h>

#include "NetworkRandomComponent.h"
#include "Multiplayer/GemSpawnerComponent.h"
#include <Multiplayer/Components/ISimplePlayerSpawner.h>


#if AZ_TRAIT_CLIENT
#   include <AzFramework/Input/Buses/Requests/InputSystemCursorRequestBus.h>
#   include <AzFramework/Input/Devices/Mouse/InputDeviceMouse.h>
#   include <LyShine/Bus/UiCursorBus.h>
#endif

#if AZ_TRAIT_SERVER
#   include <GameState/GameStateRequestBus.h>
#   include <GameState/GameStateWaitingForPlayers.h>
#   include <GameState/GameStatePreparingMatch.h>
#   include <GameState/GameStateMatchInProgress.h>
#   include <GameState/GameStateMatchEnded.h>
#endif

namespace MultiplayerSample
{
    AZ_ENUM_DEFINE_REFLECT_UTILITIES(AllowedPlayerActions);

    void NetworkMatchComponent::Reflect(AZ::ReflectContext* context)
    {
        AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context);
        if (serializeContext)
        {
            serializeContext->Class<NetworkMatchComponent, NetworkMatchComponentBase>()
                ->Version(1);
        }
        NetworkMatchComponentBase::Reflect(context);

        if (AZ::BehaviorContext* behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            behaviorContext->EBus<MultiplayerSample::NetworkMatchComponentRequestBus>("Network Match Component Requests")
                ->Attribute(AZ::Script::Attributes::Scope, AZ::Script::Attributes::ScopeFlags::Common)
                ->Attribute(AZ::Script::Attributes::Module, "multiplayersample")
                ->Attribute(AZ::Script::Attributes::Category, "MultiplayerSample")
                ->Event("Get the allowable player actions", &MultiplayerSample::NetworkMatchComponentRequestBus::Events::PlayerActionsAllowed)
                ->Event("Get roundtime remaining in seconds", &MultiplayerSample::NetworkMatchComponentRequestBus::Events::GetRoundTimeRemainingSec)
                ->Event("Get total roundtime in seconds", &MultiplayerSample::NetworkMatchComponentRequestBus::Events::GetTotalRoundTimeSec)
                ->Event("Get current round number", &MultiplayerSample::NetworkMatchComponentRequestBus::Events::GetCurrentRoundNumber)
                ->Event("Get total round count", &MultiplayerSample::NetworkMatchComponentRequestBus::Events::GetTotalRoundCount)
                ->Event("Get total player count", &MultiplayerSample::NetworkMatchComponentRequestBus::Events::GetTotalPlayerCount)
                ;
        }
    }

    void NetworkMatchComponent::OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        #if AZ_TRAIT_CLIENT
            AZ::Interface<INetworkMatch>::Register(this);
        #endif

        if (IsNetEntityRoleAuthority() || IsNetEntityRoleServer())
        {
            PlayerIdentityNotificationBus::Handler::BusConnect();
        }

        NetworkMatchComponentRequestBus::Handler::BusConnect();
    }

    void NetworkMatchComponent::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        NetworkMatchComponentRequestBus::Handler::BusDisconnect();
        PlayerIdentityNotificationBus::Handler::BusDisconnect();

        #if AZ_TRAIT_CLIENT
            AZ::Interface<INetworkMatch>::Unregister(this);
        #endif
    }

    AllowedPlayerActions NetworkMatchComponent::PlayerActionsAllowed() const
    {
#if AZ_TRAIT_CLIENT
        // Don't allow player movement if the UI cursor is visible
        bool isCursorVisible = false;
        UiCursorBus::BroadcastResult(isCursorVisible, &UiCursorInterface::IsUiCursorVisible);
        if (isCursorVisible)
        {
            return AllowedPlayerActions::None;
        }

        // Don't allow player movement if the system cursor is visible
        AzFramework::SystemCursorState systemCursorState{ AzFramework::SystemCursorState::Unknown };
        AzFramework::InputSystemCursorRequestBus::EventResult(systemCursorState, AzFramework::InputDeviceMouse::Id,
            &AzFramework::InputSystemCursorRequests::GetSystemCursorState);
        if ((systemCursorState == AzFramework::SystemCursorState::UnconstrainedAndVisible) ||
            (systemCursorState == AzFramework::SystemCursorState::ConstrainedAndVisible))
        {
            return AllowedPlayerActions::None;
        }

#endif

        // Disable player actions between rounds (rest period)
        if (GetRoundTime() <= 0 && GetRoundRestTimeRemaining() > 0)
        {
            return AllowedPlayerActions::RotationOnly;
        }

        // Disable player actions if the match hasn't started and we're still waiting for more players to join
        if ( AZ::Interface<Multiplayer::IMultiplayer>::Get()->GetCurrentHostTimeMs() < GetMatchStartHostTime())
        {
            return AllowedPlayerActions::RotationOnly;
        }

        return AllowedPlayerActions::All;
    }

    float NetworkMatchComponent::GetRoundTimeRemainingSec() const
    {
        return aznumeric_cast<float>(GetRoundTime());
    }

    float NetworkMatchComponent::GetTotalRoundTimeSec() const
    {
        return GetRoundDuration();
    }

    int32_t NetworkMatchComponent::GetCurrentRoundNumber() const
    {
        return aznumeric_cast<int32_t>(GetRoundNumber());
    }

    int32_t NetworkMatchComponent::GetTotalRoundCount() const
    {
        return aznumeric_cast<int32_t>(GetTotalRounds());
    }

    int32_t NetworkMatchComponent::GetTotalPlayerCount() const
    {
        return aznumeric_cast<int32_t>(GetPlayerCount());
    }

    AZ::TimeMs NetworkMatchComponent::GetMatchStartHostTime() const
    {
        return NetworkMatchComponentBase::GetMatchStartHostTime();
    }

    void NetworkMatchComponent::AddRoundNumberEventHandler(AZ::Event<uint16_t>::Handler& handler)
    {
        RoundNumberAddEvent(handler);
    }

    void NetworkMatchComponent::AddRoundTimeRemainingEventHandler(AZ::Event<RoundTimeSec>::Handler& handler)
    {
        RoundTimeAddEvent(handler);
    }

    void NetworkMatchComponent::AddRoundRestTimeRemainingEventHandler(AZ::Event<RoundTimeSec>::Handler& handler)
    {
        RoundRestTimeRemainingAddEvent(handler);
    }

    void NetworkMatchComponent::AddFirstMatchStartHostTime(AZ::Event<AZ::TimeMs>::Handler& handler)
    {
        this->MatchStartHostTimeAddEvent(handler);
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
        #if AZ_TRAIT_SERVER
            AZ::SimpleLcgRandom randomNumberGenerator(aznumeric_cast<int64_t>(AZ::GetElapsedTimeMs()));
            m_playerNameRandomStartingIndexPrefix = randomNumberGenerator.GetRandom() % AutoAssignedPlayerNamePrefix.size();
            m_playerNameRandomStartingIndexPostfix = randomNumberGenerator.GetRandom() % AutoAssignedPlayerNamePostfix.size();
        

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
        #endif

        PlayerMatchLifecycleBus::Handler::BusConnect();
    }

    void NetworkMatchComponentController::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        PlayerMatchLifecycleBus::Handler::BusDisconnect();

#if AZ_TRAIT_SERVER
        GameState::GameStateRequestBus::Broadcast(&GameState::GameStateRequestBus::Events::PopAllGameStates);

        GameState::GameStateRequests::RemoveGameStateFactoryOverrideForType<GameStateWaitingForPlayers>();
        GameState::GameStateRequests::RemoveGameStateFactoryOverrideForType<GameStatePreparingMatch>();
        GameState::GameStateRequests::RemoveGameStateFactoryOverrideForType<GameStateMatchInProgress>();
        GameState::GameStateRequests::RemoveGameStateFactoryOverrideForType<GameStateMatchEnded>();

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

        // Print the player results to server.log for tracking tournament winners.
        // Sort the players by score (highest score is 1st)
        // If scores are matching, then sort by remaining armor.
        AZStd::sort(results.m_playerStates.begin(), results.m_playerStates.end(), [](const PlayerState& a, const PlayerState& b)
            {
                if (a.m_score == b.m_score)
                {
                    return a.m_remainingArmor > b.m_remainingArmor;
                }
                return a.m_score > b.m_score;
            });

        AZStd::string prettyPrintMatchResults = "";
        prettyPrintMatchResults += AZStd::string::format("Match Results (%u players)\n", results.m_playerStates.size());
        for (const PlayerState& playerState : results.m_playerStates)
        {
            prettyPrintMatchResults += AZStd::string::format("\tPlayer %s score %u, armor %u.\n", playerState.m_playerName.c_str(), playerState.m_score, playerState.m_remainingArmor);
        }
        AZ_Info("NetworkMatchComponentController", prettyPrintMatchResults.c_str());


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
        // As soon as a round ends, remove all the gems until the next round begins.
        GetGemSpawnerComponentController()->RemoveGems();

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
        SetPlayerCount(aznumeric_cast<int16_t>(m_players.size()));
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
        SetPlayerCount(aznumeric_cast<int16_t>(m_players.size()));
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
                AZ::Vector3 playerTranslation = playerHandle.Exists() 
                    ? playerHandle.GetEntity()->GetTransform()->GetWorldTranslation() 
                    : AZ::Vector3::CreateZero();
                RespawnPlayer(playerEntity, PlayerResetOptions{ true, GetRespawnPenaltyPercent() });
                if (playerHandle.Exists())
                {
                    MultiplayerSample::GemSpawnerComponent* gemSpawnerComponent = GetParent().GetGemSpawnerComponent();

                    if (gemSpawnerComponent)
                    {
                        const AZStd::vector<PlayerCoinState>& coinStates = GetMatchPlayerCoinsComponentController()->GetParent().
                            GetPlayerCoinCounts();

                        const auto coinStateIterator = AZStd::find_if(
                            coinStates.begin(), coinStates.end(), [playerEntity](const PlayerCoinState& state)
                            {
                                return state.m_playerId == playerEntity;
                            });

                        if (coinStateIterator != coinStates.end())
                        {
                            float coinsDropped = coinStateIterator->m_coins * (GetRespawnPenaltyPercent() * 0.01f);

                            gemSpawnerComponent->RPC_SpawnGemWithValue(
                                playerEntity, playerTranslation, GetRespawnGemTag(), static_cast<uint16_t>(coinsDropped));
                        }
                        else
                        {
                            gemSpawnerComponent->RPC_SpawnGem(
                                playerEntity, playerTranslation, GetRespawnGemTag());
                        }
                            
                    }
                }
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

    PlayerNameString NetworkMatchComponentController::GeneratePlayerName()
    {
        // The first-name will be offset depending on how many times all prefix names have been used.
        // This has the affect of exhausting all the possible name combinations before hitting a name collision.
        const int prefixOffset = aznumeric_cast<int>(AZStd::floorf( static_cast<float>(m_nextPlayerId) / AutoAssignedPlayerNamePrefix.size()));
        const PlayerNameString prefixName = AutoAssignedPlayerNamePrefix[(++m_playerNameRandomStartingIndexPrefix + prefixOffset) % AutoAssignedPlayerNamePrefix.size()];
        const PlayerNameString postfixName = AutoAssignedPlayerNamePostfix[++m_playerNameRandomStartingIndexPostfix % AutoAssignedPlayerNamePostfix.size()];

        const PlayerNameString playerName = prefixName + postfixName;
        return playerName;
    }

    void NetworkMatchComponentController::AssignPlayerIdentity(Multiplayer::NetEntityId playerEntity)
    {
        const Multiplayer::ConstNetworkEntityHandle entityHandle = Multiplayer::GetNetworkEntityManager()->GetEntity(playerEntity);
        if (entityHandle.Exists())
        {
            if (PlayerIdentityComponent* identity = entityHandle.GetEntity()->FindComponent<PlayerIdentityComponent>())
            {
                identity->RPC_AssignPlayerName(GeneratePlayerName());
            }
            else
            {
                AZ_Warning("NetworkMatchComponentController", false, "Player entity did not have PlayerIdentityComponent");
            }
        }

        m_nextPlayerId++;
    }

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
                AZ::Transform respawnPoint = AZ::Transform::CreateIdentity();
                if (auto simplePlayerSpawner = AZ::Interface<Multiplayer::ISimplePlayerSpawner>::Get())
                {
                    respawnPoint = simplePlayerSpawner->GetNextSpawnPoint();

                    // Increment the next spawn point so any new players or respawned players don't spawn in on top of us at this location.
                    const uint32_t spawnPointCount = simplePlayerSpawner->GetSpawnPointCount();
                    simplePlayerSpawner->SetNextSpawnPointIndex((simplePlayerSpawner->GetNextSpawnPointIndex()+1) % spawnPointCount);
                }
                else
                {
                    AZ_Warning("NetworkMatchComponentController", false, "Failed to find a valid respawn point; moving to the world origin.");
                }

                teleport->Teleport(respawnPoint.GetTranslation());
            }
        }
        else
        {
            AZ_Warning("NetworkMatchComponentController", false, "Attempted respawn of an unknown player: %llu", aznumeric_cast<AZ::u64>(playerEntity));
        }
    }
#endif
}
