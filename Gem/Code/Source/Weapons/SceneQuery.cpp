/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <Source/Weapons/SceneQuery.h>
#include <AzFramework/Physics/Common/PhysicsSimulatedBody.h>
#include <AzFramework/Physics/ShapeConfiguration.h>
#include <AzFramework/Physics/PhysicsScene.h>
#include <AzFramework/Physics/Material.h>
#include <Multiplayer/IMultiplayer.h>
#include <Multiplayer/NetworkEntity/INetworkEntityManager.h>
#include <Multiplayer/NetworkTime/INetworkTime.h>

namespace MultiplayerSample
{
    namespace SceneQuery
    {
        static AZStd::shared_ptr<Physics::ShapeConfiguration> GatherShapeToPhysicsShape(const GatherShape& gatherShape, const IntersectFilter& filter)
        {
            if (gatherShape == GatherShape::Point)
            {
                // Point shape generally means a raycast, but we fall back to a small sphere in case if Overlap with Point type is requested.
                const float pointSphereSize = 0.01f;
                return AZStd::make_unique<Physics::SphereShapeConfiguration>(pointSphereSize);
            }

            // AzPhysics Scene queries work with shared_ptr
            switch (gatherShape)
            {
            case GatherShape::Box:
                AZ_Assert(filter.m_shapeConfiguration->GetShapeType() == Physics::ShapeType::Box, "Shape configuration type must be Box");
                return AZStd::make_unique<Physics::BoxShapeConfiguration>(*(azdynamic_cast<const Physics::BoxShapeConfiguration*>(filter.m_shapeConfiguration)));
            case GatherShape::Sphere:
                AZ_Assert(filter.m_shapeConfiguration->GetShapeType() == Physics::ShapeType::Sphere, "Shape configuration type must be Sphere");
                return AZStd::make_unique<Physics::SphereShapeConfiguration>(*(azdynamic_cast<const Physics::SphereShapeConfiguration*>(filter.m_shapeConfiguration)));
            case GatherShape::Capsule:
                AZ_Assert(filter.m_shapeConfiguration->GetShapeType() == Physics::ShapeType::Capsule, "Shape configuration type must be Capsule");
                return AZStd::make_unique<Physics::CapsuleShapeConfiguration>(*(azdynamic_cast<const Physics::CapsuleShapeConfiguration*>(filter.m_shapeConfiguration)));
            default:
                AZ_Warning("", false, "Only box, sphere, and capsule conversions are supported.");
            }

            return nullptr;
        }

        static void CollectHits(AzPhysics::SceneQueryHits& result, IntersectResults& outResults)
        {
            auto* networkEntityManager = AZ::Interface<Multiplayer::INetworkEntityManager>::Get();
            AZ_Assert(networkEntityManager, "Multiplayer entity manager must be initialized");

            for (const AzPhysics::SceneQueryHit& hit : result.m_hits)
            {
                IntersectResult intersectResult;
                intersectResult.m_position = hit.m_position;
                intersectResult.m_normal = hit.m_normal;
                intersectResult.m_materialName = hit.m_material->GetSurfaceTypeName();
                intersectResult.m_netEntityId = networkEntityManager->GetNetEntityIdById(hit.m_entityId);
                outResults.emplace_back(intersectResult);
            }
        }

        size_t WorldIntersect(const GatherShape& intersectShape, const IntersectFilter& filter, IntersectResults& outResults)
        {
            AZ_Assert(intersectShape == GatherShape::Point || filter.m_shapeConfiguration != nullptr,
                "Shape configuration must be provided for shape casts and overlap requests");

            auto* sceneInterface = AZ::Interface<AzPhysics::SceneInterface>::Get();
            AZ_Assert(sceneInterface, "Physics system must be initialized");

            AzPhysics::SceneHandle sceneHandle = sceneInterface->GetSceneHandle(AzPhysics::DefaultPhysicsSceneName);
            AZ_Assert(sceneHandle != AzPhysics::InvalidSceneHandle, "Default Physics world must be created");

            auto* networkEntityManager = AZ::Interface<Multiplayer::INetworkEntityManager>::Get();
            AZ_Assert(networkEntityManager, "Multiplayer entity manager must be initialized");

            auto ignoreEntitiesFilterCallback =
                [&filter, networkEntityManager](const AzPhysics::SimulatedBody* body, [[maybe_unused]] const Physics::Shape* shape)
            {
                // Exclude the bodies from another rewind frame
                if ((filter.m_rewindFrameId != Multiplayer::InvalidHostFrameId)
                     && (body->GetFrameId() != static_cast<uint32_t>(filter.m_rewindFrameId)))
                {
                    return AzPhysics::SceneQuery::QueryHitType::None;
                }

                // Find the net entity ID for this body
                AZ::EntityId bodyEntityId = body->GetEntityId();
                Multiplayer::NetEntityId bodyNetEntityId = networkEntityManager->GetNetEntityIdById(bodyEntityId);

                // Ignore the body from the filtered net entities
                if (bodyNetEntityId != Multiplayer::InvalidNetEntityId && filter.m_filteredNetEntityIds.count(bodyNetEntityId) == 1)
                {
                    // Allow static/non-net entities to hit
                    return AzPhysics::SceneQuery::QueryHitType::None;
                }

                return AzPhysics::SceneQuery::QueryHitType::Touch;
            };

            const float maxSweepDistance = filter.m_sweep.GetLength();
            const bool shouldDoOverlap = (maxSweepDistance == 0);

            // Ensure any entities that we might interact with are properly synchronized to their rewind state
            const AZ::Vector3 minBound = filter.m_initialPose.GetTranslation().GetMin(filter.m_initialPose.GetTranslation() + filter.m_sweep);
            const AZ::Vector3 maxBound = filter.m_initialPose.GetTranslation().GetMax(filter.m_initialPose.GetTranslation() + filter.m_sweep);
            const AZ::Aabb rewindBounds = AZ::Aabb::CreateFromMinMax(minBound, maxBound);
            Multiplayer::GetNetworkTime()->SyncEntitiesToRewindState(rewindBounds);

            if (shouldDoOverlap)
            {
                // Interset queries with 0 length are considered Overlaps
                AzPhysics::OverlapRequest request;
                request.m_collisionGroup = filter.m_collisionGroup;
                request.m_pose = filter.m_initialPose;
                request.m_shapeConfiguration = GatherShapeToPhysicsShape(intersectShape, filter);
                request.m_queryType = filter.m_queryType;

                // Overlap filter callback signature is slightly different from Ray/ShapeCast
                // Have to wrap it into a pass-through lambda
                request.m_filterCallback = [&ignoreEntitiesFilterCallback](const AzPhysics::SimulatedBody* body, const Physics::Shape* shape)
                {
                    return ignoreEntitiesFilterCallback(body, shape) == AzPhysics::SceneQuery::QueryHitType::None ? false : true;
                };

                AzPhysics::SceneQueryHits result = sceneInterface->QueryScene(sceneHandle, &request);
                CollectHits(result, outResults);
            }
            else if (intersectShape == GatherShape::Point)
            {
                // Perform raycast
                AzPhysics::RayCastRequest request;
                request.m_collisionGroup = filter.m_collisionGroup;
                request.m_start = filter.m_initialPose.GetTranslation();
                request.m_direction = filter.m_sweep.GetNormalized();
                request.m_distance = maxSweepDistance;
                request.m_queryType = filter.m_queryType;
                request.m_filterCallback = AZStd::move(ignoreEntitiesFilterCallback);
                request.m_reportMultipleHits = (filter.m_intersectMultiple == HitMultiple::Yes);

                AzPhysics::SceneQueryHits result = sceneInterface->QueryScene(sceneHandle, &request);
                CollectHits(result, outResults);
            }
            else
            {
                // Perform shapecast
                AzPhysics::ShapeCastRequest request;
                request.m_collisionGroup = filter.m_collisionGroup;
                request.m_start = filter.m_initialPose;
                request.m_direction = filter.m_sweep.GetNormalized();
                request.m_distance = maxSweepDistance;
                request.m_shapeConfiguration = GatherShapeToPhysicsShape(intersectShape, filter);
                request.m_queryType = filter.m_queryType;
                request.m_filterCallback = AZStd::move(ignoreEntitiesFilterCallback);
                request.m_reportMultipleHits = (filter.m_intersectMultiple == HitMultiple::Yes);

                AzPhysics::SceneQueryHits result = sceneInterface->QueryScene(sceneHandle, &request);
                CollectHits(result, outResults);
            }
            
            return outResults.size();
        }
    }
}
