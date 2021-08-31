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
    //! @class TraceWeapon
    //! @brief Weapon class for all instant hit area and trace weapons.
    class TraceWeapon final
        : public BaseWeapon
    {
    public:

        //! Constructor.
        //! @param constructParams required construction parameters for the weapon instance
        TraceWeapon(const ConstructParams& constructParams);
        ~TraceWeapon() override = default;

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
        TraceWeapon& operator =(const TraceWeapon&) = delete;
    };
}
