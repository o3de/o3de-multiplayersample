/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 * 
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <Source/Spawners/RoundRobinSpawner.h>

namespace MultiplayerSample
{
    bool RoundRobinSpawner::RegisterPlayerSpawner(NetworkPlayerSpawnerComponent* spawner)
    {
        if (!AZStd::find(m_spawners.begin(), m_spawners.end(), spawner))
        {
            m_spawners.push_back(spawner);
            return true;
        }

        return false;
    }

    AZStd::pair<Multiplayer::PrefabEntityId, AZ::Transform> RoundRobinSpawner::GetNextPlayerSpawn()
    {
        return AZStd::make_pair<Multiplayer::PrefabEntityId, AZ::Transform>(Multiplayer::PrefabEntityId(), AZ::Transform::CreateIdentity());
    }

    bool RoundRobinSpawner::UnregisterPlayerSpawner(NetworkPlayerSpawnerComponent* spawner)
    {
        if (AZStd::find(m_spawners.begin(), m_spawners.end(), spawner))
        {
            m_spawners.erase(AZStd::remove(m_spawners.begin(), m_spawners.end(), spawner));
            return true;
        }

        return false;
    }
} // namespace MultiplayerSample
