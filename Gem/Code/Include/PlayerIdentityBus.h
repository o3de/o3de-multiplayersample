/*
 * Copyright (c) Contributors to the Open 3D Engine Project
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/EBus/EBus.h>
#include <Multiplayer/MultiplayerTypes.h>

namespace MultiplayerSample
{
    class PlayerIdentityNotifications : public AZ::EBusTraits
    {
    public:
        virtual ~PlayerIdentityNotifications() = default;

        virtual void OnPlayerActivated([[maybe_unused]] Multiplayer::NetEntityId playerEntity) {}
        virtual void OnPlayerDeactivated([[maybe_unused]] Multiplayer::NetEntityId playerEntity) {}
    };

    using PlayerIdentityNotificationBus = AZ::EBus<PlayerIdentityNotifications>;
}
