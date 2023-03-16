/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <Source/Weapons/WeaponGathers.h>
#include <Multiplayer/NetworkTime/INetworkTime.h>
#include <AzFramework/Physics/PhysicsScene.h>
#include <AzCore/Component/Component.h>
#include <AzCore/Console/ILogger.h>
#include <Source/Weapons/SceneQuery.h>

#if AZ_TRAIT_CLIENT
#include <DebugDraw/DebugDrawBus.h>
#endif

namespace MultiplayerSample
{
    AZ_CVAR(uint32_t, bg_MultitraceNumTraceSegments, 3, nullptr, AZ::ConsoleFunctorFlags::Null, "The number of segments to use when performing multitrace casts");
    AZ_CVAR(bool, bg_DrawPhysicsRaycasts, false, nullptr, AZ::ConsoleFunctorFlags::Null, "If enabled, will debug draw physics raycasts");

    IntersectFilter::IntersectFilter
    (
        const AZ::Transform& initialPose, 
        const AZ::Vector3& sweep, 
        AzPhysics::SceneQuery::QueryType queryType,
        HitMultiple intersectMultiple,
        const AzPhysics::CollisionGroup& collisionGroup,
        const NetEntityIdSet& filteredNetEntityIds,
        const Physics::ShapeConfiguration* shapeConfiguration
    )
        : m_initialPose(initialPose)
        , m_sweep(sweep)
        , m_queryType(queryType)
        , m_intersectMultiple(intersectMultiple)
        , m_collisionGroup(collisionGroup)
        , m_filteredNetEntityIds(filteredNetEntityIds)
        , m_shapeConfiguration(shapeConfiguration)
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
        IntersectResults& outResults
    )
    {
        const AZ::Transform& startTransform = eventData.m_initialTransform;
        const AZ::Vector3    sweep = eventData.m_targetPosition - startTransform.GetTranslation();
        const HitMultiple    hitMultiple = gatherParams.m_multiHit ? HitMultiple::Yes : HitMultiple::No;
        const GatherShape&   intersectShape = gatherParams.m_gatherShape;
        AzPhysics::CollisionGroup collisionGroup = AzPhysics::GetCollisionGroupById(gatherParams.m_collisionGroupId);

        IntersectFilter filter(startTransform, sweep, AzPhysics::SceneQuery::QueryType::StaticAndDynamic, hitMultiple,
            collisionGroup, filteredNetEntityIds, gatherParams.GetCurrentShapeConfiguration());
        SceneQuery::WorldIntersect(intersectShape, filter, outResults);

#if AZ_TRAIT_CLIENT
        if (bg_DrawPhysicsRaycasts)
        {
            DebugDraw::DebugDrawRequestBus::Broadcast
            (
                &DebugDraw::DebugDrawRequests::DrawLineLocationToLocation,
                eventData.m_initialTransform.GetTranslation(),
                eventData.m_targetPosition,
                AZ::Colors::Red,
                10.0f
            );
        }
#endif

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
        AzPhysics::SceneInterface* sceneInterface = AZ::Interface<AzPhysics::SceneInterface>::Get();
        AzPhysics::SceneHandle sceneHandle = sceneInterface->GetSceneHandle(AzPhysics::DefaultPhysicsSceneName);
        const AZ::Vector3& gravity = gatherParams.m_bulletDrop ? sceneInterface->GetGravity(sceneHandle) : AZ::Vector3::CreateZero();
        const float segmentTickSize = deltaTime / bg_MultitraceNumTraceSegments; // Duration in seconds of each cast segment
        const AZ::Vector3 segmentStepOffset = sweep * gatherParams.m_travelSpeed; // Displacement (disregarding gravity) of our bullet over one second
        const float maxTravelDistanceSq = gatherParams.m_castDistance * gatherParams.m_castDistance;

        // We're not doing any lift or drift computations due to the magnus effects a bullet is subject to
        // Any such adjustments, estimates for how fast the bullet is spinning due to muzzle exit velocity and the rifling of the gun, air density, temperature, etc...

        const HitMultiple hitMultiple = gatherParams.m_multiHit ? HitMultiple::Yes : HitMultiple::No;
        const AzPhysics::CollisionGroup collisionGroup = AzPhysics::GetCollisionGroupById(gatherParams.m_collisionGroupId);

        float currSegmentStartTime = inOutActiveShot.m_lifetimeSeconds;
        AZ::Vector3 currSegmentPosition = inOutActiveShot.m_initialTransform.GetTranslation() + (segmentStepOffset * currSegmentStartTime) + (gravity * 0.5f * currSegmentStartTime * currSegmentStartTime);
        for (uint32_t segment = 0; segment < bg_MultitraceNumTraceSegments; ++segment)
        {
            float nextSegmentStartTime = currSegmentStartTime + segmentTickSize;
            AZ::Vector3 travelDistance = (segmentStepOffset * nextSegmentStartTime); // Total distance our shot has traveled as of this cast, ignoring arc-length due to gravity
            AZ::Vector3 nextSegmentPosition = inOutActiveShot.m_initialTransform.GetTranslation() + travelDistance + (gravity * 0.5f * nextSegmentStartTime * nextSegmentStartTime);

            const AZ::Transform currSegTransform = AZ::Transform::CreateLookAt(currSegmentPosition, nextSegmentPosition);
            const AZ::Vector3 segSweep = nextSegmentPosition - currSegmentPosition;

            IntersectFilter filter(currSegTransform, segSweep, AzPhysics::SceneQuery::QueryType::StaticAndDynamic,
                hitMultiple, collisionGroup, filteredNetEntityIds, gatherParams.GetCurrentShapeConfiguration());
            SceneQuery::WorldIntersect(gatherParams.m_gatherShape, filter, outResults);

#if AZ_TRAIT_CLIENT
            if (bg_DrawPhysicsRaycasts)
            {
                DebugDraw::DebugDrawRequestBus::Broadcast
                (
                    &DebugDraw::DebugDrawRequests::DrawLineLocationToLocation,
                    currSegmentPosition,
                    nextSegmentPosition,
                    segment % 2 == 0 ? AZ::Colors::Red : AZ::Colors::Yellow,
                    10.0f
                );
            }
#endif

            // Terminate the loop if we hit something
            if (((outResults.size() > 0) && !gatherParams.m_multiHit) || (travelDistance.GetLengthSq() > maxTravelDistanceSq))
            {
                result = ShotResult::ShouldTerminate;
#if AZ_TRAIT_CLIENT
                if (bg_DrawPhysicsRaycasts && !outResults.empty())
                {
                    DebugDraw::DebugDrawRequestBus::Broadcast
                    (
                        &DebugDraw::DebugDrawRequests::DrawSphereAtLocation,
                        outResults[0].m_position,
                        /*radius=*/0.1f,
                        AZ::Colors::Green,
                        /*duration=*/10.0f
                    );
                }
#endif
                break;
            }

            currSegmentStartTime = nextSegmentStartTime;
            currSegmentPosition = nextSegmentPosition;
        }

        inOutActiveShot.m_lifetimeSeconds = LifetimeSec(inOutActiveShot.m_lifetimeSeconds + deltaTime);
        return result;
    }
}
