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

#pragma once

#include <Source/Weapons/WeaponTypes.h>
#include <AzCore/std/containers/unordered_set.h>

namespace MultiplayerSample
{
    typedef AZStd::unordered_set<Multiplayer::NetEntityId> NetEntityIdSet;

    enum class HitStatic { No, Yes };
    enum class HitDynamic { No, Yes };
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
            HitStatic intersectStatic, 
            HitDynamic intersectDynamic, 
            HitMultiple intersectMultiple, 
            const NetEntityIdSet& filteredEntityIds
        );

        Multiplayer::HostFrameId m_rewindFrameId = Multiplayer::InvalidHostFrameId; // If an entity is dynamic, it must be synced to this frameId to pass intersect testing
        AZ::Transform            m_initialPose;
        AZ::Vector3              m_sweep;
        HitStatic                m_intersectStatic;
        HitDynamic               m_intersectDynamic;
        HitMultiple              m_intersectMultiple;
        NetEntityIdSet           m_filteredNetEntityIds;

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
