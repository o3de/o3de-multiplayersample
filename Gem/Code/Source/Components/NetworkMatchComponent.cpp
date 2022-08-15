/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <MultiplayerSampleTypes.h>
#include <UiGameOverBus.h>
#include <Source/Components/Multiplayer/MatchPlayerCoinsComponent.h>
#include <Source/Components/Multiplayer/PlayerIdentityComponent.h>
#include <Source/Components/NetworkMatchComponent.h>

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
        if (IsNetEntityRoleAuthority() || IsNetEntityRoleServer())
        {
            PlayerIdentityNotificationBus::Handler::BusConnect();
        }
    }

    void NetworkMatchComponent::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        PlayerIdentityNotificationBus::Handler::BusDisconnect();
    }

    void NetworkMatchComponent::OnPlayerActivated(Multiplayer::NetEntityId playerEntity)
    {
        RPC_PlayerActivated(playerEntity);
    }

    void NetworkMatchComponent::OnPlayerDeactivated(Multiplayer::NetEntityId playerEntity)
    {
        RPC_PlayerDeactivated(playerEntity);
    }

    void NetworkMatchComponent::HandleRPC_EndMatch(
        [[maybe_unused]] AzNetworking::IConnection* invokingConnection, [[maybe_unused]] const MatchResultsSummary& results)
    {
        if (IsNetEntityRoleClient())
        {
            UiGameOverBus::Broadcast(&UiGameOverBus::Events::SetGameOverScreenEnabled, true);
            UiGameOverBus::Broadcast(&UiGameOverBus::Events::DisplayResults, results);
        }
    }

    // Controller methods

    NetworkMatchComponentController::NetworkMatchComponentController(NetworkMatchComponent& parent)
        : NetworkMatchComponentControllerBase(parent)
    {
    }

    void NetworkMatchComponentController::OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        SetRoundTime(RoundTimeSec{ GetRoundDuration() });
        SetRoundNumber(1);

        // Tick once a second, this way we can keep the time as an 2 byte integer instead of a float.
        m_roundTickEvent.Enqueue(AZ::TimeMs{ 1000 }, true);
    }

    void NetworkMatchComponentController::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        m_roundTickEvent.RemoveFromQueue();
    }

    void NetworkMatchComponentController::EndMatch()
    {
        //Signal event to end the match
        m_roundTickEvent.RemoveFromQueue();

        MatchResultsSummary results;

        const AZStd::vector<PlayerCoinState>& coinStates = GetMatchPlayerCoinsComponentController()->GetParent().
            GetPlayerCoinCounts();

        int highestCoins = -1;

        for (const Multiplayer::NetEntityId playerNetEntity : m_players)
        {
            PlayerState state;
            const auto playerHandle = Multiplayer::GetNetworkEntityManager()->GetEntity(playerNetEntity);
            if (playerHandle.Exists())
            {
                if (const PlayerIdentityComponent* identity = playerHandle.GetEntity()->FindComponent<PlayerIdentityComponent>())
                {
                    state.m_playerName = identity->GetPlayerName();
                }
                if (const NetworkHealthComponent* armor = playerHandle.GetEntity()->FindComponent<NetworkHealthComponent>())
                {
                    // Treating health as armor
                    state.m_remainingShield = aznumeric_cast<uint8_t>(armor->GetHealth());
                }
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
                    results.m_winningPlayerName = state.m_playerName;
                }
            }

            results.m_playerStates.push_back(state);
        }

        RPC_EndMatch(results);
    }

    void NetworkMatchComponentController::EndRound()
    {
        uint16_t& roundNumber = ModifyRoundNumber();
        if (roundNumber < GetTotalRounds())
        {
            //Signal event to reset everything
            ++roundNumber;
            SetRoundTime(RoundTimeSec{ GetRoundDuration() });
        }
        else
        {
            //Signal event to end match
            EndMatch();
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

    void NetworkMatchComponentController::RoundTickOnceASecond()
    {
        // m_roundTickEvent is configured to tick once a second
        SetRoundTime(RoundTimeSec(GetRoundTime() - 1.f));
        
        if (GetRoundTime() <= RoundTimeSec(0.f))
        {
            EndRound();
        }
    }

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
}
