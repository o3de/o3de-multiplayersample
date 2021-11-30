/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <Source/Weapons/ProjectileWeapon.h>

namespace MultiplayerSample
{
    ProjectileWeapon::ProjectileWeapon(const ConstructParams& constructParams)
        : BaseWeapon(constructParams)
    {
        ;
    }

    void ProjectileWeapon::Activate
    (
        [[maybe_unused]] WeaponState& weaponState,
        [[maybe_unused]] const Multiplayer::ConstNetworkEntityHandle weaponOwner,
        [[maybe_unused]] ActivateEvent& eventData,
        [[maybe_unused]] bool validateActivation
    )
    {
        ; // need to port this code
    }

    void ProjectileWeapon::TickActiveShots([[maybe_unused]] WeaponState& weaponState, [[maybe_unused]] float deltaTime)
    {
        ; // no-op, projectiles spawn as individual entities that tick themselves..  if a game does client steered projectiles then this pattern may need to change
    }
}
