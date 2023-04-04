/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <GameState/GameStateRequestBus.h>
#include <Source/GameState/GameStatePreparingMatch.h>
#include <Source/GameState/GameStateWaitingForPlayers.h>
#include <PlayerMatchLifecycleBus.h>

namespace MultiplayerSample
{
    AZ_CVAR(uint32_t, sv_MpsFirstMatchDelaySeconds, 0, nullptr, AZ::ConsoleFunctorFlags::DontReplicate,
        "Controls how many seconds the server waits to start the first match after the first player has connected. "
        "This is a cvar instead of network archetype because it's important for server admins to be able to set this as a launch parameter instead of having to update a level prefab. This setting could be removed if using holding lobby and matchmaking."
        "For example, during a tournament, if all the players are ready a server admin can start server with a 60 second match delay.");


    GameStateWaitingForPlayers::GameStateWaitingForPlayers([[maybe_unused]] NetworkMatchComponentController* controller)
        : m_controller(controller)
    {
        PlayerIdentityNotificationBus::Handler::BusConnect();
    }

    void GameStateWaitingForPlayers::OnPlayerActivated([[maybe_unused]] Multiplayer::NetEntityId playerEntity)
    {
        PlayerIdentityNotificationBus::Handler::BusDisconnect();

        // The first player has joined, start the timer before starting the first match
        const AZ::TimeMs firstMatchDelayMs = AZ::SecondsToTimeMs(sv_MpsFirstMatchDelaySeconds);
        const AZ::TimeMs firstMatchHostTime = AZ::Interface<Multiplayer::IMultiplayer>::Get()->GetCurrentHostTimeMs() + firstMatchDelayMs;

        m_controller->SetMatchStartHostTime(firstMatchHostTime);
        m_beginMatchEvent.Enqueue(firstMatchDelayMs);
    }

    void GameStateWaitingForPlayers::BeginMatch()
    {
        const auto state = GameState::GameStateRequests::CreateNewOverridableGameStateOfType<GameStatePreparingMatch>();
        GameState::GameStateRequestBus::Broadcast(&GameState::GameStateRequestBus::Events::ReplaceActiveGameState, state);
    }
}
