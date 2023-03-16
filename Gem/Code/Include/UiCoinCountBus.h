/*
 * Copyright (c) Contributors to the Open 3D Engine Project
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/EBus/EBus.h>

namespace MultiplayerSample
{
    class UiCoinCountNotifications : public AZ::EBusTraits
    {
    public:
        AZ_RTTI(UiCoinCountNotifications, "{cb316d00-cdb6-4e2e-97ea-ca2d74145eb2}");
        virtual ~UiCoinCountNotifications() = default;

        virtual void OnCoinCountChanged([[maybe_unused]] uint16_t totalCoinsCollectedByLocalPlayer) {}
    };

    using UiCoinCountNotificationBus = AZ::EBus<UiCoinCountNotifications>;
}
