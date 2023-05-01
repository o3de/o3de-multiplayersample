/*
 * Copyright (c) Contributors to the Open 3D Engine Project
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/Component/ComponentBus.h>
#include <AzCore/Math/Vector3.h>

namespace MultiplayerSample
{
    class PlayerKnockbackNotifications : public AZ::ComponentBus
    {
    public:
        ~PlayerKnockbackNotifications() override = default;

        virtual void OnPlayerKnockback(
            [[maybe_unused]] const AZ::Vector3& fromPosition,
            [[maybe_unused]] const AZ::Vector3& PlayerKnockbackDirection) {}
    };

    using PlayerKnockbackNotificationBus = AZ::EBus<PlayerKnockbackNotifications>;
}
