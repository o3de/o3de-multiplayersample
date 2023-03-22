/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <GameplayEffectsNotificationBus.h>
#include <GameState/GameStateRequestBus.h>
#include <Source/GameState/GameStateMatchEnded.h>
#include <Source/GameState/GameStatePreparingMatch.h>

namespace MultiplayerSample
{    
    GameStateMatchEnded::GameStateMatchEnded([[maybe_unused]] NetworkMatchComponentController* controller)
    {
#if AZ_TRAIT_SERVER
        m_controller = controller;	    
#endif	    
    }

    void GameStateMatchEnded::OnEnter()
    {
#if AZ_TRAIT_SERVER
        m_controller->EndMatch();
#endif
        m_finishingTime = AZ::TimeMs{ 3000 };
        m_finishingEvent.Enqueue(AZ::Time::ZeroTimeMs, true);

        GameplayEffectsNotificationBus::Broadcast(&GameplayEffectsNotificationBus::Events::OnEffect, SoundEffect::GameEnd);
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
