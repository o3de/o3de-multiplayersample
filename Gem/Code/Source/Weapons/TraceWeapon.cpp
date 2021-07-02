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
        float deltaTime,
        WeaponState& weaponState,
        [[maybe_unused]] const Multiplayer::ConstNetworkEntityHandle weaponOwner,
        ActivateEvent& eventData,
        bool dispatchHitEvents,
        bool dispatchActivateEvents,
        bool forceSkipGather
    )
    {
        const bool validate = dispatchHitEvents; // Perform activation validation if we're actually going to dispatch hit events
        if (ActivateInternal(weaponState, validate))
        {
            if (!dispatchHitEvents)
            {
                // Skip out on all the remaining activation logic if this is a replay.. We do not want to spawn particles or sounds or other effects during a replay event
                return;
            }

            if (dispatchActivateEvents)
            {
                m_weaponListener.OnActivate(WeaponActivationInfo(*this, eventData));
            }

            const bool isMultiSegmented = (m_weaponParams.m_gatherParams.m_travelSpeed > 0.0f);

            IntersectResults gatherResults;
            if (isMultiSegmented)
            {
                ActiveShot activeShot{ eventData.m_initialTransform, eventData.m_targetPosition, LifetimeSec{ 0.0f } };

                const ShotResult result = GatherEntitiesMultisegment(deltaTime, activeShot, gatherResults);

                // If this activation did not immediately terminate this frame, then push back the new shot to track
                if (result == ShotResult::ShouldTerminate)
                {
                    DispatchHitEvents(gatherResults, eventData, m_gatheredNetEntityIds);
                }
                else
                {
                    weaponState.m_activeShots.emplace_back(activeShot);
                }
            }
            else if (!forceSkipGather)
            {
                if (GatherEntities(eventData, gatherResults))
                {
                    DispatchHitEvents(gatherResults, eventData, m_gatheredNetEntityIds);
                }
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
