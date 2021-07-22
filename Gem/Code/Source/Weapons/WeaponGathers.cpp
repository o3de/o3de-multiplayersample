/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <Source/Weapons/WeaponGathers.h>
#include <Multiplayer/NetworkTime/INetworkTime.h>

namespace MultiplayerSample
{
    AZ_CVAR(uint32_t, bg_MultitraceNumTraceSegments, 3, nullptr, AZ::ConsoleFunctorFlags::Null, "The number of segments to use when performing multitrace casts");

    IntersectFilter::IntersectFilter
    (
        const AZ::Transform& initialPose, 
        const AZ::Vector3& sweep, 
        HitStatic intersectStatic, 
        HitDynamic intersectDynamic, 
        HitMultiple intersectMultiple, 
        const NetEntityIdSet& filteredNetEntityIds
    )
        : m_initialPose(initialPose)
        , m_sweep(sweep)
        , m_intersectStatic(intersectStatic)
        , m_intersectDynamic(intersectDynamic)
        , m_intersectMultiple(intersectMultiple)
        , m_filteredNetEntityIds(filteredNetEntityIds)
    {
        Multiplayer::INetworkTime* networkTime = Multiplayer::GetNetworkTime();
        if (networkTime->IsTimeRewound())
        {
            m_rewindFrameId = networkTime->GetHostFrameId();
        }
    }

    bool GatherEntities
    (
        const GatherParams& gatherParams, 
        const ActivateEvent& eventData, 
        const NetEntityIdSet& filteredNetEntityIds, 
        [[maybe_unused]] IntersectResults& outResults
    )
    {
        const AZ::Transform& startTransform = eventData.m_initialTransform;
        const AZ::Vector3    sweep = eventData.m_targetPosition - startTransform.GetTranslation();
        const HitMultiple    hitMultiple = gatherParams.m_multiHit ? HitMultiple::Yes : HitMultiple::No;
        [[maybe_unused]] const GatherShape&   intersectShape = gatherParams.m_gatherShape;

        IntersectFilter filter(startTransform, sweep, HitStatic::Yes, HitDynamic::Yes, hitMultiple, filteredNetEntityIds);
        //gNovaGame->GetNetworkPhysicalWorld().WorldIntersect(intersectShape, filter, outResultList);

        return true;
    }

    ShotResult GatherEntitiesMultisegment
    (
        const GatherParams& gatherParams, 
        const NetEntityIdSet& filteredNetEntityIds, 
        float deltaTime, 
        ActiveShot& inOutActiveShot, 
        IntersectResults& outResults
    )
    {
        // This only works when our cast is not instantaneous (it requires some positive, non-zero travel speed)
        AZ_Assert(gatherParams.m_travelSpeed > 0.0f, "GatherEntitiesMultiSegment called with an invalid travel speed! This will fail, use the non-segmented gather path instead.");

        ShotResult result = ShotResult::DoNotTerminate;

        const AZ::Transform& startTransform = inOutActiveShot.m_initialTransform;
        const AZ::Vector3 sweep = (inOutActiveShot.m_targetPosition - startTransform.GetTranslation()).GetNormalized();

        // World gravity for our current location (making the currently safe assumption that it's constant over the duration of our trace)
        //const AZ::Vector3& gravity = gatherParams.m_bulletDrop ? gNovaGame->GetNetworkPhysicalWorld().GetWorldGravity(a_InOutActiveShot.GetInitialTransform().m_Position) : AZ::Vector3::CreateZero();
        const AZ::Vector3& gravity = AZ::Vector3::CreateZero();
        const float segmentTickSize = deltaTime / bg_MultitraceNumTraceSegments; // Duration in seconds of each cast segment
        const AZ::Vector3 segmentStepOffset = sweep * gatherParams.m_travelSpeed; // Displacement (disregarding gravity) of our bullet over one second
        const float maxTravelDistanceSq = gatherParams.m_castDistance * gatherParams.m_castDistance;

        // We're not doing any lift or drift computations due to the magnus effects a bullet is subject to
        // Any such adjustments, estimates for how fast the bullet is spinning due to muzzle exit velocity and the rifling of the gun, air density, temperature, etc...

        const HitMultiple hitMultiple = gatherParams.m_multiHit ? HitMultiple::Yes : HitMultiple::No;

        float currSegmentStartTime = inOutActiveShot.m_lifetimeSeconds;
        AZ::Vector3 currSegmentPosition = inOutActiveShot.m_initialTransform.GetTranslation() + (segmentStepOffset * currSegmentStartTime) + (gravity * 0.5f * currSegmentStartTime * currSegmentStartTime);
        for (uint32_t segment = 0; segment < bg_MultitraceNumTraceSegments; ++segment)
        {
            float nextSegmentStartTime = currSegmentStartTime + segmentTickSize;
            AZ::Vector3 travelDistance = (segmentStepOffset * nextSegmentStartTime); // Total distance our shot has traveled as of this cast, ignoring arc-length due to gravity
            AZ::Vector3 nextSegmentPosition = inOutActiveShot.m_initialTransform.GetTranslation() + travelDistance + (gravity * 0.5f * nextSegmentStartTime * nextSegmentStartTime);

            const AZ::Transform currSegTransform = AZ::Transform::CreateFromQuaternionAndTranslation(inOutActiveShot.m_initialTransform.GetRotation(), currSegmentPosition);
            const AZ::Vector3   segSweep = nextSegmentPosition - currSegmentPosition;

            IntersectFilter filter(currSegTransform, segSweep, HitStatic::Yes, HitDynamic::Yes, hitMultiple, filteredNetEntityIds);
            //gNovaGame->GetNetworkPhysicalWorld().WorldIntersect(gatherParams.m_gatherShape, filter, a_OutResultList);

            // Terminate the loop if we hit something
            if (((outResults.size() > 0) && !gatherParams.m_multiHit) || (travelDistance.GetLengthSq() > maxTravelDistanceSq))
            {
                result = ShotResult::ShouldTerminate;
                break;
            }

            currSegmentStartTime = nextSegmentStartTime;
            currSegmentPosition = nextSegmentPosition;
        }

        inOutActiveShot.m_lifetimeSeconds = LifetimeSec(inOutActiveShot.m_lifetimeSeconds + deltaTime);

        return result;
    }
}
