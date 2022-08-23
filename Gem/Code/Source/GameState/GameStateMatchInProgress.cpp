/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AzCore/Settings/SettingsRegistry.h>
#include <GameState/GameStateRequestBus.h>
#include <Source/GameState/GameStateMatchInProgress.h>

namespace MultiplayerSample
{
    GameStateMatchInProgress::GameStateMatchInProgress(NetworkMatchComponentController* controller)
        : m_controller(controller)
    {
        if (const auto registry = AZ::SettingsRegistry::Get())
        {
            registry->Get(m_winningCoinCount, WinningCoinCountSetting);
        }
    }

    void GameStateMatchInProgress::OnEnter()
    {
        PlayerCoinCollectorNotificationBus::Handler::BusConnect();
        if (m_controller)
        {
            m_controller->RoundNumberAddEvent(m_roundChangedHandler);
            m_controller->StartMatch();
        }
    }

    void GameStateMatchInProgress::OnExit()
    {
        PlayerCoinCollectorNotificationBus::Handler::BusDisconnect();
        m_roundChangedHandler.Disconnect();
    }

    void GameStateMatchInProgress::OnPlayerCollectedCoinCountChanged([[maybe_unused]] Multiplayer::NetEntityId playerEntity,
        uint16_t coinsCollected)
    {
        if (coinsCollected >= m_winningCoinCount)
        {
            const auto state = GameState::GameStateRequests::CreateNewOverridableGameStateOfType<GameStateMatchEnded>();
            GameState::GameStateRequestBus::Broadcast(&GameState::GameStateRequestBus::Events::ReplaceActiveGameState, state);
        }
    }

    int GameStateMatchInProgress::GetNotificationOrder()
    {
        // Putting this state as the last handler so that the player states are updated, otherwise the winning count might be off.
        return LastNotificationOrder;
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
