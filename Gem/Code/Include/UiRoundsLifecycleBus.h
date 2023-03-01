/*
 * Copyright (c) Contributors to the Open 3D Engine Project
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/Component/ComponentBus.h>
#include <MultiplayerSampleTypes.h>

namespace MultiplayerSample
{
    class UiRoundsLifecycleNotifications : public AZ::ComponentBus
    {
    public:
        virtual ~UiRoundsLifecycleNotifications() = default;

        //! The seconds remaining in the rest period before the next round starts
        virtual void OnRoundRestTimeRemainingChanged(RoundTimeSec secondsRenaming) = 0;
    };

    using UiRoundsLifecycleBus = AZ::EBus<UiRoundsLifecycleNotifications>;
}