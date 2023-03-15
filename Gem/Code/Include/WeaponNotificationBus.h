/*
 * Copyright (c) Contributors to the Open 3D Engine Project
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/EBus/EBus.h>
#include <Source/MultiplayerSampleTypes.h>

namespace MultiplayerSample
{
    class WeaponNotifications : public AZ::EBusTraits
    {
    public:
        virtual ~WeaponNotifications() = default;

        //! Called on a local player that has been confirmed to hit a player with a weapon.
        virtual void OnConfirmedHitPlayer([[maybe_unused]] AZ::EntityId byPlayerEntity, [[maybe_unused]] AZ::EntityId otherPlayerEntity) {}
    };

    using WeaponNotificationBus = AZ::EBus<WeaponNotifications>;
}
