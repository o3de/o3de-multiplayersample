/*
 * Copyright (c) Contributors to the Open 3D Engine Project
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/Preprocessor/Enum.h>
#include <AzCore/Component/ComponentBus.h>
#include <AzCore/Math/Vector3.h>
#include <Source/MultiplayerSampleTypes.h>

namespace MultiplayerSample
{
    class GameplayEffectsNotifications : public AZ::EBusTraits
    {
    public:
        virtual ~GameplayEffectsNotifications() = default;

        //! Plays at the a specified position on clients.
        virtual void OnPositionalEffect([[maybe_unused]] SoundEffect effect, [[maybe_unused]] const AZ::Vector3& position) {}

        //! Non-positional effect, played at the local player camera's position.
        virtual void OnEffect([[maybe_unused]] SoundEffect effect) {}
    };

    using GameplayEffectsNotificationBus = AZ::EBus<GameplayEffectsNotifications>;

    using LocalOnlyGameplayEffectsNotificationBus = AZ::EBus<GameplayEffectsNotifications>;
}
