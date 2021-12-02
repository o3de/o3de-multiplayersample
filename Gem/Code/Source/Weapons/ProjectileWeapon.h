/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <Source/Weapons/BaseWeapon.h>

namespace MultiplayerSample
{
    //! @class ProjectileWeapon
    //! @brief Server-side weapon class for projectile-based weapons.
    class ProjectileWeapon final
        : public BaseWeapon
    {
    public:

        //! Constructor.
        //! @param constructParams required construction parameters for the weapon instance
        ProjectileWeapon(const ConstructParams& constructParams);
        ~ProjectileWeapon() override = default;

    private:

        //! IWeapon interface
        //! @{
        void Activate
        (
            WeaponState& weaponState,
            const Multiplayer::ConstNetworkEntityHandle weaponOwner,
            ActivateEvent& eventData,
            bool validateActivation
        ) override;

        void TickActiveShots(WeaponState& weaponState, float deltaTime) override;
        //! @}

        // Do not allow assignment
        ProjectileWeapon &operator =(const ProjectileWeapon &) = delete;
    };
}
