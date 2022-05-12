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

    AZStd::pair<AZ::Data::Asset<AzFramework::Spawnable>, AZ::Transform> RoundRobinSpawner::GetNextPlayerSpawn()
    {
        if (m_spawners.empty())
        {
            AZLOG_WARN("No active NetworkPlayerSpawnerComponents were found on player spawn request.")
            return AZStd::make_pair(AZ::Data::Asset<AzFramework::Spawnable>(), AZ::Transform::CreateIdentity());
        }

        const NetworkPlayerSpawnerComponent* spawner = m_spawners[m_spawnIndex];
        m_spawnIndex = m_spawnIndex + 1 == m_spawners.size() ? 0 : m_spawnIndex + 1;

        return AZStd::make_pair(spawner->GetSpawnableAsset(), spawner->GetEntity()->GetTransform()->GetWorldTM());
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
