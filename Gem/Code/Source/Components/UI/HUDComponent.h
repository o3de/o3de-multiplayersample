/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/Component/Component.h>
#include <AzCore/EBus/Event.h>
#include <Source/Components/NetworkMatchComponent.h>

#include "MultiplayerSampleTypes.h"

namespace MultiplayerSample
{
    class HUDComponent
        : public AZ::Component
    {
    public:
        AZ_COMPONENT(MultiplayerSample::HUDComponent, "{8061E5D2-A1F7-4B40-9AAC-8FF14BD094FC}");

        /*
        * Reflects component data into the reflection contexts, including the serialization, edit, and behavior contexts.
        */
        static void Reflect(AZ::ReflectContext* context);

    protected:
        void Activate() override;
        void Deactivate() override;

    private:
        // Wait for NetworkMatchComponent to activate so we can begin listening for NetworkMatch events
        AZ::ScheduledEvent m_waitForActiveNetworkMatchComponent = AZ::ScheduledEvent([this]
            {
                if (const auto networkMatchComponent = AZ::Interface<INetworkMatch>::Get())
                {
                    m_waitForActiveNetworkMatchComponent.RemoveFromQueue();

                    SetRoundNumberText(aznumeric_cast<uint16_t>(networkMatchComponent->GetCurrentRoundNumber()));
                    m_roundNumberHandler = AZ::EventHandler<uint16_t>([this](uint16_t value) { SetRoundNumberText(value); });
                    networkMatchComponent->AddRoundNumberEventHandler(m_roundNumberHandler);

                    m_roundTimerHandler = AZ::EventHandler<RoundTimeSec>([this](RoundTimeSec value) { SetRoundTimerText(value); });
                    networkMatchComponent->AddRoundTimeRemainingEventHandler(m_roundTimerHandler);

                    // Listen for an event if the host changes the match start time
                    // This can happen if an admin changes the start time during a tournament to wait for more players.
                    UpdateFirstMatchTimerUi();
                    m_firstMatchStartHostTimeHandler = AZ::EventHandler<AZ::TimeMs>([this]([[maybe_unused]]AZ::TimeMs value) {UpdateFirstMatchTimerUi(); });
                    networkMatchComponent->AddFirstMatchStartHostTime(m_firstMatchStartHostTimeHandler);
                }
            }, AZ::Name("HUDComponent Wait For Active NetworkMatchComponent"));

        void SetRoundNumberText(uint16_t round);
        void SetRoundTimerText(RoundTimeSec time);
        void UpdateFirstMatchTimerUi();

        AZ::EventHandler<uint16_t> m_roundNumberHandler; 
        AZ::EventHandler<RoundTimeSec> m_roundTimerHandler;
        AZ::EventHandler<AZ::TimeMs> m_firstMatchStartHostTimeHandler;
        AZ::ScheduledEvent m_updateFirstMatchTimer = AZ::ScheduledEvent([this]
            {
                UpdateFirstMatchTimerUi();
            }, AZ::Name("HUDComponent Update First Match Timer"));

        AZ::EntityId m_roundNumberUi;
        AZ::EntityId m_roundTimerUi;
        AZStd::string m_roundNumberText;
        AZ::EntityId m_roundSecondsRemainingUiParent;
        AZ::EntityId m_firstMatchStartingUiParent;
        AZ::EntityId m_firstMatchStartingTimerUi;
    };
} // namespace MultiplayerSample
