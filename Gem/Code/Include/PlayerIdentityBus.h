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
        virtual void OnAutonomousPlayerNameChanged([[maybe_unused]] const char* playerName) {}
    };

    using PlayerIdentityNotificationBus = AZ::EBus<PlayerIdentityNotifications>;

    class PlayerIdentityRequests : public AZ::EBusTraits
    {
    public:
        virtual ~PlayerIdentityRequests() = default;
        
        virtual const char* GetPlayerIdentityName() = 0;
    };

    using PlayerIdentityRequestBus = AZ::EBus<PlayerIdentityRequests>;
}
