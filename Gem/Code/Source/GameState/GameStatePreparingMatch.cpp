/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <GameplayEffectsNotificationBus.h>
#include <GameState/GameStateRequestBus.h>
#include <Source/GameState/GameStateMatchInProgress.h>
#include <Source/GameState/GameStatePreparingMatch.h>

namespace MultiplayerSample
{
    GameStatePreparingMatch::GameStatePreparingMatch(NetworkMatchComponentController* controller)
        : m_controller(controller)
    {
    }

    void GameStatePreparingMatch::OnEnter()
    {
        m_preparationTime = AZ::TimeMs{ 3000 };
        m_preparingEvent.Enqueue(AZ::Time::ZeroTimeMs, true);

        GameplayEffectsNotificationBus::Broadcast(&GameplayEffectsNotificationBus::Events::OnEffect, SoundEffect::CountDown);
    }

    void GameStatePreparingMatch::OnExit()
    {
        m_preparingEvent.RemoveFromQueue();
    }

    void GameStatePreparingMatch::OnPreparingMatchTick()
    {
        m_preparationTime -= m_preparingEvent.TimeInQueueMs();
        if (m_preparationTime <= AZ::Time::ZeroTimeMs)
        {
            m_preparingEvent.RemoveFromQueue();

            const auto state = GameState::GameStateRequests::CreateNewOverridableGameStateOfType<GameStateMatchInProgress>();
            GameState::GameStateRequestBus::Broadcast(&GameState::GameStateRequestBus::Events::ReplaceActiveGameState, state);
        }
    }
}
