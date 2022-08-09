/*
 * Copyright (c) Contributors to the Open 3D Engine Project
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/Component/EntityId.h>
#include <AzCore/EBus/EBus.h>
#include <Multiplayer/MultiplayerTypes.h>

namespace MultiplayerSample
{
    class PlayerCoinCollectorNotifications : public AZ::EBusTraits
    {
    public:
        virtual ~PlayerCoinCollectorNotifications() = default;

        virtual void OnPlayerCollectorActivated([[maybe_unused]] Multiplayer::NetEntityId playerEntity) {}
        virtual void OnPlayerCollectorDeactivated([[maybe_unused]] Multiplayer::NetEntityId playerEntity) {}
        virtual void OnPlayerCollectedCoinCountChanged([[maybe_unused]] Multiplayer::NetEntityId playerEntity, [[maybe_unused]] uint16_t coinsCollected) {}
    };

    using PlayerCoinCollectorNotificationBus = AZ::EBus<PlayerCoinCollectorNotifications>;
}
