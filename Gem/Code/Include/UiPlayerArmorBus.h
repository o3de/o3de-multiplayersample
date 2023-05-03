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
    class UiPlayerArmorNotifications : public AZ::EBusTraits
    {
    public:
        AZ_RTTI(UiPlayerArmorNotifications, "{cb316d00-cdb6-4e2e-97ea-ca2d74145eb2}");
        virtual ~UiPlayerArmorNotifications() = default;

        virtual void OnPlayerArmorChanged([[maybe_unused]] float armorPointsForLocalPlayer, [[maybe_unused]] float startingArmor) {}
    };

    using UiPlayerArmorNotificationBus = AZ::EBus<UiPlayerArmorNotifications>;
}
