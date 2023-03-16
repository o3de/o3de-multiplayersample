/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/EBus/EBus.h>
#include <Multiplayer/MultiplayerTypes.h>

namespace MultiplayerSample
{
    class PlayerMatchLifecycleNotifications : public AZ::EBusTraits
    {
    public:
        virtual ~PlayerMatchLifecycleNotifications() = default;

        virtual void OnPlayerArmorZero([[maybe_unused]] Multiplayer::NetEntityId playerEntity) {}
    };

    using PlayerMatchLifecycleBus = AZ::EBus<PlayerMatchLifecycleNotifications>;
}