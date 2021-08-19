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
#include <Multiplayer/NetworkEntity/INetworkEntityManager.h>

namespace MultiplayerSample
{
    namespace SceneQuery
    {
        static AZStd::shared_ptr<Physics::ShapeConfiguration> GatherShapeToPhysicsShape(const GatherShape& gatherShape)
        {
            const float defaultValue = 1.0f; // TODO: Pass via gather params

            switch (gatherShape)
            {
            case GatherShape::Box:
                return AZStd::make_unique<Physics::BoxShapeConfiguration>(AZ::Vector3(defaultValue));
                break;
            case GatherShape::Sphere:
                return AZStd::make_unique<Physics::SphereShapeConfiguration>(defaultValue);
                break;
            case GatherShape::Capsule:
                return AZStd::make_unique<Physics::CapsuleShapeConfiguration>(defaultValue, defaultValue);
                break;
            default:
                AZ_Warning("", false, "Only box, sphere, and capsule conversions are supported.");
                return nullptr;
            }

        }

        static AzPhysics::SceneQuery::QueryType GetQueryTypeFromFilter(const IntersectFilter& filter)
        {
            // There's no "None" type for the scene query type, using "Static" by default.
            AzPhysics::SceneQuery::QueryType queryType = AzPhysics::SceneQuery::QueryType::Static;

            if (filter.m_intersectStatic == HitStatic::Yes && filter.m_intersectDynamic == HitDynamic::Yes)
            {
                queryType = AzPhysics::SceneQuery::QueryType::StaticAndDynamic;
            }
            else if (filter.m_intersectDynamic == HitDynamic::Yes)
            {
                queryType = AzPhysics::SceneQuery::QueryType::Dynamic;
            }

            return queryType;
        }

        static void CollectHits(AzPhysics::SceneQueryHits& result, IntersectResults& outResults)
        {
            auto* networkEntityManager = AZ::Interface<Multiplayer::INetworkEntityManager>::Get();

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

        void WorldIntersect(const GatherShape& intersectShape, const IntersectFilter& filter, IntersectResults& outResults)
        {
            auto* sceneInterface = AZ::Interface<AzPhysics::SceneInterface>::Get();
            AZ_Assert(sceneInterface, "Physics system must be initialized");

            AzPhysics::SceneHandle sceneHandle = sceneInterface->GetSceneHandle(AzPhysics::DefaultPhysicsSceneName);
            AZ_Assert(sceneHandle != AzPhysics::InvalidSceneHandle, "Default Physics world must be created");

            auto* networkEntityManager = AZ::Interface<Multiplayer::INetworkEntityManager>::Get();
            AZ_Assert(networkEntityManager, "Multiplayer entity manager must be initialized");

            AzPhysics::SceneQuery::QueryType queryType = GetQueryTypeFromFilter(filter);

            auto ignoreEntitiesFilterCallback =
                [&filter, networkEntityManager](const AzPhysics::SimulatedBody* body, [[maybe_unused]] const Physics::Shape* shape)
            {
                // Exclude the bodies from another rewind frame
                if (body->GetFrameId() != static_cast<uint32_t>(filter.m_rewindFrameId))
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

            if (intersectShape == GatherShape::Point)
            {
                // Perform raycast
                AzPhysics::RayCastRequest request;
                request.m_collisionGroup = filter.m_collisionGroup;
                request.m_start = filter.m_initialPose.GetTranslation();
                request.m_direction = filter.m_sweep.GetNormalized();
                request.m_distance = filter.m_sweep.GetLength();
                request.m_queryType = queryType;
                request.m_filterCallback = AZStd::move(ignoreEntitiesFilterCallback);

                AzPhysics::SceneQueryHits result = sceneInterface->QueryScene(sceneHandle, &request);
                CollectHits(result, outResults);
            }
            else
            {
                AzPhysics::ShapeCastRequest request;
                request.m_collisionGroup = filter.m_collisionGroup;
                request.m_start = filter.m_initialPose;
                request.m_direction = filter.m_sweep.GetNormalized();
                request.m_distance = filter.m_sweep.GetLength();
                request.m_shapeConfiguration = GatherShapeToPhysicsShape(intersectShape);
                request.m_queryType = queryType;
                request.m_filterCallback = AZStd::move(ignoreEntitiesFilterCallback);

                AzPhysics::SceneQueryHits result = sceneInterface->QueryScene(sceneHandle, &request);
                CollectHits(result, outResults);
            }
        }
    }
}
