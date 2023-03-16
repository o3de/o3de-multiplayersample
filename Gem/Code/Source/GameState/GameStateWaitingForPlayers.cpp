/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <GameState/GameStateRequestBus.h>
#include <Source/GameState/GameStatePreparingMatch.h>
#include <Source/GameState/GameStateWaitingForPlayers.h>

namespace MultiplayerSample
{
    GameStateWaitingForPlayers::GameStateWaitingForPlayers([[maybe_unused]] NetworkMatchComponentController* controller)
    {
        PlayerIdentityNotificationBus::Handler::BusConnect();
    }

    void GameStateWaitingForPlayers::OnPlayerActivated([[maybe_unused]] Multiplayer::NetEntityId playerEntity)
    {
        PlayerIdentityNotificationBus::Handler::BusDisconnect();
        
        const auto state = GameState::GameStateRequests::CreateNewOverridableGameStateOfType<GameStatePreparingMatch>();
        GameState::GameStateRequestBus::Broadcast(&GameState::GameStateRequestBus::Events::ReplaceActiveGameState, state);
    }
}
