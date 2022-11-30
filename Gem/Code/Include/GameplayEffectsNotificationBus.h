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
    AZ_ENUM_CLASS(SoundEffect,
        // Player Sounds
        PlayerFootSteps,
        PlayerExertion,
        PlayerKnockedDown,
        ArmorBreaking,
        ArmorMend,
        PlayerOuch,
        LadderClimb,
        ShutDown,

        // Game Event Sounds
        CountDown,
        GemPickup,
        VictoryFanfare,
        LosingFanfare,
        RoundStart,
        RoundEnd,
        GameEnd,

        // Laser Pistol
        LaserPistolMuzzleFlash,
        LaserPistolImpact,

        // Bubble Gun
        BubbleGunBuildup,
        BubbleGunMuzzleFlash,
        BubbleGunProjectile,
        BubbleGunImpact,

        // Jump Pad
        JumpPadLaunch,

        // Energy Ball Trap
        EnergyBallTrapRisingOutOfTheGround,
        EnergyBallTrapBuildup, // followed by muzzle flash
        EnergyBallTrapProjectile,
        EnergyBallTrapImpact,
        EnergyBallTrapOnCooldown, // plays when you try to fire it during cooldown
    );

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
}
