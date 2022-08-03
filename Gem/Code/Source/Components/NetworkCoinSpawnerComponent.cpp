/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <Source/Components/NetworkCoinSpawnerComponent.h>

#include "PerfTest/NetworkPrefabSpawnerComponent.h"

namespace MultiplayerSample
{
    NetworkCoinSpawnerComponentController::NetworkCoinSpawnerComponentController(NetworkCoinSpawnerComponent& parent)
        : NetworkCoinSpawnerComponentControllerBase(parent)
    {
    }

    void NetworkCoinSpawnerComponentController::OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        PrefabCallbacks callbacks;
        callbacks.m_onActivateCallback = [this](AZStd::shared_ptr<AzFramework::EntitySpawnTicket> ticket,
            AzFramework::SpawnableConstEntityContainerView view)
        {
            for (const AZ::Entity* entity : view)
            {
                if (entity->FindComponent<NetworkCoinComponent>())
                {
                    // Save the coin spawn ticket, otherwise the coin will de-spawn
                    m_spawnedCoins.emplace(entity, AZStd::move(ticket));
                    break;
                }
            }
        };

        GetParent().GetNetworkPrefabSpawnerComponent()->SpawnPrefabAsset(GetEntity()->GetTransform()->GetWorldTM(),
            GetParent().GetCoinSpawnable(), AZStd::move(callbacks));
    }

    void NetworkCoinSpawnerComponentController::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
    }
}
