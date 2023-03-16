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
    class PlayerCoinCollectorNotifications : public AZ::EBusTraits
    {
    public:
        virtual ~PlayerCoinCollectorNotifications() = default;

        static constexpr AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::MultipleAndOrdered;

        //! Determines the order in which handlers receive coin events.
        struct BusHandlerOrderCompare
        {
            bool operator()(PlayerCoinCollectorNotifications* left, PlayerCoinCollectorNotifications* right) const
            {
                return left->GetNotificationOrder() < right->GetNotificationOrder();
            }
        };

        virtual void OnPlayerCollectorActivated([[maybe_unused]] Multiplayer::NetEntityId playerEntity) {}
        virtual void OnPlayerCollectorDeactivated([[maybe_unused]] Multiplayer::NetEntityId playerEntity) {}
        virtual void OnPlayerCollectedCoinCountChanged([[maybe_unused]] Multiplayer::NetEntityId playerEntity, [[maybe_unused]] uint16_t coinsCollected) {}

        virtual int GetNotificationOrder() { return FirstNotificationOrder; }

        static constexpr int LastNotificationOrder = 1000;
        static constexpr int FirstNotificationOrder = 0;
    };

    using PlayerCoinCollectorNotificationBus = AZ::EBus<PlayerCoinCollectorNotifications>;
}
