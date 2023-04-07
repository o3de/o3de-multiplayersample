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
#include <AzCore/Time/ITime.h>

namespace MultiplayerSample
{    
    GameStateMatchEnded::GameStateMatchEnded([[maybe_unused]] NetworkMatchComponentController* controller)
        : m_controller(controller)
    {
    }

    void GameStateMatchEnded::OnEnter()
    {
        m_controller->EndMatch();

        const AZ::TimeMs restBeforeNewMatch = AZ::SecondsToTimeMs(m_controller->GetRestDurationBetweenMatches());
        const AZ::TimeMs nextMatchStartTime = AZ::Interface<Multiplayer::IMultiplayer>::Get()->GetCurrentHostTimeMs() + restBeforeNewMatch;
        m_controller->SetMatchStartHostTime(nextMatchStartTime);
        m_finishingEvent.Enqueue(restBeforeNewMatch);

        GameplayEffectsNotificationBus::Broadcast(&GameplayEffectsNotificationBus::Events::OnEffect, SoundEffect::GameEnd);
    }

    void GameStateMatchEnded::OnExit()
    {
        m_finishingEvent.RemoveFromQueue();
    }

    void GameStateMatchEnded::OnFinishedMatch()
    {
        const auto state = GameState::GameStateRequests::CreateNewOverridableGameStateOfType<GameStatePreparingMatch>();
        GameState::GameStateRequestBus::Broadcast(&GameState::GameStateRequestBus::Events::ReplaceActiveGameState, state);
    }
}
