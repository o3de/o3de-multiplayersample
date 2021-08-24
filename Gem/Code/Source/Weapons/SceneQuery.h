/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <Source/Weapons/WeaponGathers.h>

namespace MultiplayerSample
{
    namespace SceneQuery
    {
        //! Performs a world intersection query
        //! @param intersectShape a convex shape to use for the intersection test (point, box, sphere, capsule)
        //! @param filter parameters controlling whether the query is swept, how many entities to gather, world positions, and filtering information
        //! @param a_OutResults result structure to store all relevant hits
        //! @return the number of hits stored in the result structure
        size_t WorldIntersect(const GatherShape& intersectShape, const IntersectFilter& filter, IntersectResults& outResults);
    }
}
