/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once


#include <MultiplayerSampleTypes.h>
#include <AzCore/Component/Component.h>
#include <AzCore/Component/TickBus.h>

namespace MultiplayerSample
{
    class UiRestBetweenRoundsComponent
        : public AZ::Component
        , public AZ::TickBus::Handler
    {
    public:
        AZ_COMPONENT(UiRestBetweenRoundsComponent, "{8BF185B2-DCE7-462B-B151-43E0AF717BA5}");

        static void Reflect(AZ::ReflectContext* context);

        void Activate() override;
        void Deactivate() override;

        //! AZ::TickBus::Handler overrides...
        void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;

    private:
        void OnRoundRestTimeRemainingChanged(RoundTimeSec secondsRemaining);
        AZ::Event<RoundTimeSec>::Handler onRestTimeChangedHandler{ [this](RoundTimeSec secondsRemaining)
        {
            OnRoundRestTimeRemainingChanged(secondsRemaining);
        } };

        AZ::EntityId m_restTimerRootUiElement;
        AZ::EntityId m_numbersContainerUiElement;
    };
}