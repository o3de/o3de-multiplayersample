/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/Component/Component.h>
#include <AzCore/EBus/ScheduledEvent.h>
#include <AzCore/std/containers/vector.h>
#include <Components/NetworkMatchComponent.h>
#include <StartingPointInput/InputEventNotificationBus.h>

namespace MultiplayerSample
{
    class UiMatchPlayerCoinCountsComponent
        : public AZ::Component
        , public StartingPointInput::InputEventNotificationBus::MultiHandler
    {
    public:
        static constexpr float SecondsBeforeNewRoundToHideUI = 3.0f;

        AZ_COMPONENT(UiMatchPlayerCoinCountsComponent, "{529b9b3b-bea2-4120-9089-c4451438e4c0}");

        static void Reflect(AZ::ReflectContext* context);

        void Activate() override;
        void Deactivate() override;

        //! StartingPointInput::InputEventNotificationBus overrides ...
        //! @{
        void OnPressed(float value) override;
        void OnReleased(float value) override;
        //! @}

        //! Show or hide the player score menu
        //! @param enable true will display the menu; false will hide the menu. 
        void EnableUI(bool enable);

    private:
        AZ::EntityId m_rootElementId;
        AZStd::vector<AZ::EntityId> m_playerRowElement;

        static PlayerNameString GetPlayerName(Multiplayer::NetEntityId playerEntity);

        void UpdatePlayerScoreUI();
        AZ::Event<int32_t, PlayerCoinState>::Handler m_onPlayerScoreChanged{[this](int32_t, PlayerCoinState)
        {
            UpdatePlayerScoreUI();
        } };

        // Wait for NetworkMatchComponent to activate so we can begin listening for NetworkMatch events
        // For example: when the round resets to 1 we know the new match has started.
        AZ::ScheduledEvent m_waitForActiveNetworkMatchComponent = AZ::ScheduledEvent([this]
            {
                if (const auto networkMatchComponent = AZ::Interface<INetworkMatch>::Get())
                {
                    networkMatchComponent->AddRoundTimeRemainingEventHandler(m_roundTimerHandler);
                    m_waitForActiveNetworkMatchComponent.RemoveFromQueue();
                }
            }, AZ::Name("GameOverUI Wait For Active NetworkMatchComponent"));

        // Listen for rest time between rounds coming to an end
        // Automatically close the player score menu when the round is about to start
        AZ::Event<RoundTimeSec>::Handler m_restTimerHandler{ [this](RoundTimeSec secondsRemaining)
        {
            if (secondsRemaining <= SecondsBeforeNewRoundToHideUI)
            {
                EnableUI(false);
                m_restTimerHandler.Disconnect();
            }
        } };

        // Listen for the round coming to an end
        // Automatically open up the player score menu in between rounds
        AZ::Event<RoundTimeSec>::Handler m_roundTimerHandler{ [this](RoundTimeSec secondsRemaining)
        {
            const auto networkMatch = AZ::Interface<INetworkMatch>::Get();
            if (secondsRemaining <= 0 && 
                networkMatch->GetCurrentRoundNumber() <= networkMatch->GetTotalRoundCount())
            {
                EnableUI(true);
                networkMatch->AddRoundRestTimeRemainingEventHandler(m_restTimerHandler);
            }
        } };

    };
} // namespace MultiplayerSample
