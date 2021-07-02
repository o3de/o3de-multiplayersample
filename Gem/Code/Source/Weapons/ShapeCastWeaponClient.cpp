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



#include "StdAfx.h"
#include "Weapons/ShapeCastWeaponClient.h"
#include "Physics/NetworkPhysicalWorldQueries.h"
#include "Environment/Logger.h"


using namespace NovaNet;


ShapeCastWeaponClient::ShapeCastWeaponClient(const BaseWeapon::ConstructParams& a_ConstructParams, BaseWeaponClientListener& a_Listener)
:   BaseWeaponClient(a_ConstructParams, a_Listener)
{
    ;
}


ShapeCastWeaponClient::~ShapeCastWeaponClient()
{
    ;
}


void ShapeCastWeaponClient::ActivateClient(float a_DeltaTime, WeaponState& a_WeaponState, bool a_Predicted, bool a_Replay, ActivateEvent& a_EventData)
{
    const bool bValidate = a_Predicted; // Validate only on predicted activations, since simulated guys activate off the server authority

    if (ActivateInternal(a_WeaponState, bValidate, a_EventData))
    {
        if (a_Replay)
        {
            // Skip out on all the remaining activation logic if this is a replay..  We do not want to spawn particles or sounds or other effects during a replay event
            return;
        }

        m_Listener.OnActivate(ClientActivationInfo(*this, a_EventData));

        if (a_Predicted)
        {
            const bool isMultiSegmented = (m_Params.GetGatherParams().GetTravelSpeed() > 0.0f);

            EntityIdSet emptySet;
            IntersectResults gatherResults;
            if (isMultiSegmented)
            {
                ActiveShot activeShot(a_EventData.GetInitialTransform(), a_EventData.GetTargetPosition(), LifetimeSec(0.0f));

                const EShotResult result = GatherEntitiesMultiSegment(m_Params.GetGatherParams(), m_GatheredEntities, a_DeltaTime, activeShot, gatherResults);

                if (result == EShotResult::ShouldTerminate)
                {
                    DispatchHitEvents(gatherResults, a_EventData, emptySet);
                }
                else
                {
                    // If this activation did not immediately terminate this frame, then push back the new shot to track
                    a_WeaponState.ModifyActiveShots().PushBack(activeShot);
                }
            }
            else
            {
                if (GatherEntitiesInternal(a_DeltaTime, a_EventData, gatherResults))
                {
                    DispatchHitEvents(gatherResults, a_EventData, emptySet);
                }
            }
        }
    }
}


void ShapeCastWeaponClient::TickActiveShots(WeaponState& a_WeaponState, float a_DeltaTime)
{
    uint32_t numActiveShots = a_WeaponState.GetActiveShots().GetSize();

    for (uint32_t i = 0; i < numActiveShots; ++i)
    {
        ActiveShot& activeShot = a_WeaponState.ModifyActiveShots()[i];

        IntersectResults gatherResults;
        const EShotResult result = GatherEntitiesMultiSegment(m_Params.GetGatherParams(), m_GatheredEntities, a_DeltaTime, activeShot, gatherResults);

        // If expired, dispatch hit events, swap and pop
        if (result == EShotResult::ShouldTerminate)
        {
            EntityIdSet emptySet;
            ActivateEvent eventData(activeShot.GetInitialTransform(), activeShot.GetTargetPosition(), INVALID_ENTITY_ID, INVALID_ENTITY_ID);
            DispatchHitEvents(gatherResults, eventData, emptySet);

            a_WeaponState.ModifyActiveShots()[i] = a_WeaponState.ModifyActiveShots()[numActiveShots - 1];
            a_WeaponState.ModifyActiveShots().PopBack();
            --numActiveShots;
            --i; // We have just inserted a new element into the i'th position, next iteration we now need to revisit this index
        }
    }
}
