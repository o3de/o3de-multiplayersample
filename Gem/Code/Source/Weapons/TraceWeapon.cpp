/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <Source/Weapons/TraceWeapon.h>

namespace MultiplayerSample
{
    TraceWeapon::TraceWeapon(const ConstructParams& constructParams)
        : BaseWeapon(constructParams)
    {
        ;
    }

    void TraceWeapon::Activate
    (
        WeaponState& weaponState,
        [[maybe_unused]] const Multiplayer::ConstNetworkEntityHandle weaponOwner,
        ActivateEvent& eventData,
        bool validateActivation
    )
    {
        if (ActivateInternal(weaponState, validateActivation))
        {
            m_weaponListener.OnWeaponActivate(WeaponActivationInfo(*this, eventData));

            const bool isMultiSegmented = (m_weaponParams.m_gatherParams.m_travelSpeed > 0.0f);

            IntersectResults gatherResults;
            if (isMultiSegmented)
            {
                ActiveShot activeShot{ eventData.m_initialTransform, eventData.m_targetPosition, LifetimeSec{ 0.0f } };
                if (weaponState.m_activeShots.size() < weaponState.m_activeShots.max_size())
                { 
                    weaponState.m_activeShots.emplace_back(activeShot);
                }
                else
                {
                    AZ_Assert(false, "Attempting to add too many active shots to the TraceWeapon.");
                }
            }
            else if (GatherEntities(eventData, gatherResults))
            {
                DispatchHitEvents(gatherResults, eventData, m_gatheredNetEntityIds);
            }
        }
    }

    void TraceWeapon::TickActiveShots(WeaponState& weaponState, float deltaTime)
    {
        AZStd::size_t numActiveShots = weaponState.m_activeShots.size();
        for (AZStd::size_t i = 0; i < numActiveShots; ++i)
        {
            ActiveShot& activeShot = weaponState.m_activeShots[i];

            IntersectResults gatherResults;
            const ShotResult result = GatherEntitiesMultisegment(deltaTime, activeShot, gatherResults);

            // If expired, dispatch hit events, swap and pop
            if (result == ShotResult::ShouldTerminate)
            {
                ActivateEvent eventData{ activeShot.m_initialTransform, activeShot.m_targetPosition, Multiplayer::InvalidNetEntityId, Multiplayer::InvalidNetEntityId };
                DispatchHitEvents(gatherResults, eventData, m_gatheredNetEntityIds);

                weaponState.m_activeShots[i] = weaponState.m_activeShots[numActiveShots - 1];
                weaponState.m_activeShots.pop_back();
                --numActiveShots;
                --i; // We have just inserted a new element into the i'th position, next iteration we now need to revisit this index
            }
        }
    }
}
