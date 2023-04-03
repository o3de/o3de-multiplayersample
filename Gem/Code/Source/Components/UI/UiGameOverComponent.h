/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once


#include <AzCore/Component/Component.h>
#include <Source/Components/NetworkMatchComponent.h>

#include <UiGameOverBus.h>

namespace MultiplayerSample
{
    class UiGameOverComponent
        : public AZ::Component
        , public UiGameOverBus::Handler
    {
    public:
        AZ_COMPONENT(UiGameOverComponent, "{37a2de13-a8fa-4ee1-8652-e17253137f62}");

        static void Reflect(AZ::ReflectContext* context);

        void Activate() override;
        void Deactivate() override;

        //! UiGameOverBus overrides
        //! @{
        void SetGameOverScreenEnabled(bool enabled) override;
        void DisplayResults(MatchResultsSummary results) override;
        //! }@

    private:
        // Every second the GameOver menu is open, display the time remaining until the new match begins
        // There aren't any network events around match restart time;
        // Real time is controlled on the server within the match state machine, but not shared across network.
        // See GameStatePreparingMatch.cpp and GameStateMatchEnded.cpp.
        void DisplaySecondsRemainingUI();
        AZ::ScheduledEvent m_onSecondsRemainingChanged = AZ::ScheduledEvent( [this]()
        {
            DisplaySecondsRemainingUI();

        }, AZ::Name("GameOverUI Seconds Remaining"));

        // Listen for the NetworkMatch Round Number to Change
        // Round 1 is the 1st round in a match; turn off this game-over screen.
        AZ::Event<uint16_t>::Handler m_onRoundNumberChangedHandler{ [this](uint16_t roundNumber)
        {
            if (roundNumber == 1)
            {
                SetGameOverScreenEnabled(false);
            }
        } };

        // Wait for NetworkMatchComponent to activate so we can begin listening for NetworkMatch events
        // For example: when the round resets to 1 we know the new match has started.
        AZ::ScheduledEvent m_waitForActiveNetworkMatchComponent = AZ::ScheduledEvent([this]
        {
            if (const auto networkMatchComponent = AZ::Interface<INetworkMatch>::Get())
            {
                networkMatchComponent->AddRoundNumberEventHandler(m_onRoundNumberChangedHandler);
                m_waitForActiveNetworkMatchComponent.RemoveFromQueue();
            }
        }, AZ::Name("GameOverUI Wait For Active NetworkMatchComponent"));


        AZ::EntityId m_gameOverRootElement;
        AZ::EntityId m_rankNumbersUIContainer;
        AZStd::vector<AZ::EntityId> m_topRankPlayersUIElements;
        AZ::EntityId m_timeRemainingUntilNewMatchUIContainer;
    };
}
