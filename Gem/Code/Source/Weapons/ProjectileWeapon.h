/*
 * All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
 * its licensors.
 *
 * For complete copyright and license terms please see the LICENSE at the root of this
 * distribution (the "License"). All use of this software is governed by the License,
 * or, if provided, by the license below or the license accompanying this file. Do not
 * remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
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

        //! BaseWeapon interface
        //! @{
        void Activate
        (
            float deltaTime,
            WeaponState& weaponState,
            const Multiplayer::ConstNetworkEntityHandle weaponOwner,
            ActivateEvent& eventData,
            bool dispatchHitEvents,
            bool dispatchActivateEvents,
            bool forceSkipGather
        ) override;

        void TickActiveShots(WeaponState& weaponState, float deltaTime) override;
        //! @}

        // Do not allow assignment
        ProjectileWeapon &operator =(const ProjectileWeapon &) = delete;
    };
}
