/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 * 
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AzCore/Component/TransformBus.h>
#include <Source/Components/NetworkPlayerSpawnerComponent.h>
#include <Source/Spawners/RoundRobinSpawner.h>

namespace MultiplayerSample
{
    bool RoundRobinSpawner::RegisterPlayerSpawner(NetworkPlayerSpawnerComponent* spawner)
    {
        if (AZStd::find(m_spawners.begin(), m_spawners.end(), spawner) == m_spawners.end())
        {
            m_spawners.push_back(spawner);
            return true;
        }

        return false;
    }

    AZStd::pair<Multiplayer::PrefabEntityId, AZ::Transform> RoundRobinSpawner::GetNextPlayerSpawn()
    {
        if (m_spawners.empty())
        {
            AZLOG_WARN("No active NetworkPlayerSpawnerComponents were found on player spawn request.")
            return AZStd::make_pair<Multiplayer::PrefabEntityId, AZ::Transform>(Multiplayer::PrefabEntityId(), AZ::Transform::CreateIdentity());
        }

        if (m_spawnIndex >= m_spawners.size())
        {
            AZLOG_WARN("RoundRobinSpawner has an out-of-bounds spawner index. Resetting spawn index to 0. Did you forget to call UnregisterPlayerSpawner?")
            m_spawnIndex = 0;
        }

        NetworkPlayerSpawnerComponent* spawner = m_spawners[m_spawnIndex];
        m_spawnIndex = m_spawnIndex + 1 == m_spawners.size() ? 0 : m_spawnIndex + 1;
        // NetworkEntityManager currently operates against/validates AssetId or Path, opt for Path via Hint
        Multiplayer::PrefabEntityId prefabEntityId(AZ::Name(spawner->GetSpawnableAsset().GetHint().c_str()));

        return AZStd::make_pair<Multiplayer::PrefabEntityId, AZ::Transform>(
            prefabEntityId, spawner->GetEntity()->GetTransform()->GetWorldTM());
    }

    bool RoundRobinSpawner::UnregisterPlayerSpawner(NetworkPlayerSpawnerComponent* spawner)
    {
        if (AZStd::find(m_spawners.begin(), m_spawners.end(), spawner))
        {
            m_spawners.erase(AZStd::remove(m_spawners.begin(), m_spawners.end(), spawner));

            // A spawner was removed, reset the next spawnIndex if it's now out-of-bounds
            if (m_spawnIndex >= m_spawners.size())
            {
                m_spawnIndex = 0;
            }

            return true;
        }

        return false;
    }
} // namespace MultiplayerSample
