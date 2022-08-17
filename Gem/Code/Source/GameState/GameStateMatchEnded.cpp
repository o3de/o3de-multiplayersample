/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <GameState/GameStateRequestBus.h>
#include <Source/GameState/GameStateMatchEnded.h>
#include <Source/GameState/GameStatePreparingMatch.h>

namespace MultiplayerSample
{
    GameStateMatchEnded::GameStateMatchEnded(NetworkMatchComponentController* controller)
        : m_controller(controller)
    {
    }

    void GameStateMatchEnded::OnEnter()
    {
        m_controller->EndMatch();
        m_finishingTime = AZ::TimeMs{ 3000 };
        m_finishingEvent.Enqueue(AZ::Time::ZeroTimeMs, true);
    }

    void GameStateMatchEnded::OnExit()
    {
        m_finishingEvent.RemoveFromQueue();
    }

    void GameStateMatchEnded::OnFinishedMatchTick()
    {
        m_finishingTime -= m_finishingEvent.TimeInQueueMs();
        if (m_finishingTime <= AZ::Time::ZeroTimeMs)
        {
            m_finishingEvent.RemoveFromQueue();

            const auto state = GameState::GameStateRequests::CreateNewOverridableGameStateOfType<GameStatePreparingMatch>();
            GameState::GameStateRequestBus::Broadcast(&GameState::GameStateRequestBus::Events::ReplaceActiveGameState, state);
        }
    }
}
