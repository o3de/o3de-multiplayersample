/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <Source/Weapons/WeaponTypes.h>
#include <AzCore/std/containers/unordered_set.h>
#include <AzFramework/Physics/Collision/CollisionGroups.h>

namespace MultiplayerSample
{
    typedef AZStd::unordered_set<Multiplayer::NetEntityId> NetEntityIdSet;

    enum class HitMultiple { No, Yes };

    enum class ShotResult
    {
        ShouldTerminate,
        DoNotTerminate
    };

    //! @struct IntersectFilter
    //! @brief Helper structure that defines how a world intersection query (raytrace, shapecast) should gather entities.
    struct IntersectFilter
    {
        IntersectFilter
        (
            const AZ::Transform& initialPose, 
            const AZ::Vector3& sweep, 
            AzPhysics::SceneQuery::QueryType queryType,
            HitMultiple intersectMultiple,
            const AzPhysics::CollisionGroup& collisionGroup,
            const NetEntityIdSet& filteredEntityIds,
            const Physics::ShapeConfiguration* shapeConfiguration = nullptr
        );

        Multiplayer::HostFrameId m_rewindFrameId = Multiplayer::InvalidHostFrameId; // If an entity is dynamic, it must be synced to this frameId to pass intersect testing
        AZ::Transform            m_initialPose;
        AZ::Vector3              m_sweep;
        AzPhysics::SceneQuery::QueryType m_queryType; // Intersect static, dynamic or both

        HitMultiple              m_intersectMultiple;
        NetEntityIdSet           m_filteredNetEntityIds;
        AzPhysics::CollisionGroup m_collisionGroup;
        const Physics::ShapeConfiguration* m_shapeConfiguration = nullptr; // Shape configuration for shape casts and overlaps

        IntersectFilter& operator=(const IntersectFilter&) = delete;
    };

    //! @struct IntersectResult
    //! @brief Helper structure that contains a single world intersect query result.
    struct IntersectResult
    {
        AZ::Vector3 m_position;
        AZ::Vector3 m_normal;
        Multiplayer::NetEntityId m_netEntityId;
        AZ::Name m_materialName;
    };

    //! @struct IntersectResult
    //! @brief Helper structure that holds all results from a world intersect query.
    using IntersectResults = AZStd::vector<IntersectResult>;

    bool GatherEntities
    (
        const GatherParams&   gatherParams, 
        const ActivateEvent&  eventData,
        const NetEntityIdSet& filteredNetEntityIds,
        IntersectResults&     outResults
    );

    ShotResult GatherEntitiesMultisegment
    (
        const GatherParams&   gatherParams, 
        const NetEntityIdSet& filteredNetEntityIds, 
        float                 deltaTime, 
        ActiveShot&           inOutActiveShot, 
        IntersectResults&     outResults
    );
}
