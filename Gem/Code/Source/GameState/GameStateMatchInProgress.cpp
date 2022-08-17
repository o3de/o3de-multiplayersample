/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <GameState/GameStateRequestBus.h>
#include <Source/GameState/GameStateMatchInProgress.h>

namespace MultiplayerSample
{
    GameStateMatchInProgress::GameStateMatchInProgress(NetworkMatchComponentController* controller)
        : m_controller(controller)
    {
    }

    void GameStateMatchInProgress::OnEnter()
    {
        if (m_controller)
        {
            m_controller->RoundNumberAddEvent(m_roundChangedHandler);
            m_controller->StartMatch();
        }
    }

    void GameStateMatchInProgress::OnExit()
    {
        m_roundChangedHandler.Disconnect();
    }

    void GameStateMatchInProgress::OnRoundChanged(AZ::u16 round)
    {
        if (round > m_controller->GetTotalRounds())
        {
            const auto state = GameState::GameStateRequests::CreateNewOverridableGameStateOfType<GameStateMatchEnded>();
            GameState::GameStateRequestBus::Broadcast(&GameState::GameStateRequestBus::Events::ReplaceActiveGameState, state);
        }
    }
}
