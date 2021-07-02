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
        [[maybe_unused]] float deltaTime,
        [[maybe_unused]] WeaponState& weaponState,
        [[maybe_unused]] const Multiplayer::ConstNetworkEntityHandle weaponOwner,
        [[maybe_unused]] ActivateEvent& eventData,
        [[maybe_unused]] bool dispatchHitEvents,
        [[maybe_unused]] bool dispatchActivateEvents,
        [[maybe_unused]] bool forceSkipGather
    )
    {
        ; // need to port this code
    }

    void ProjectileWeapon::TickActiveShots([[maybe_unused]] WeaponState& weaponState, [[maybe_unused]] float deltaTime)
    {
        ; // no-op, projectiles spawn as individual entities that tick themselves..  if a game does client steered projectiles then this pattern may need to change
    }
}
