/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <Source/AutoGen/NetworkCoinSpawnerComponent.AutoComponent.h>

namespace MultiplayerSample
{
    class NetworkCoinSpawnerComponentController
        : public NetworkCoinSpawnerComponentControllerBase
    {
    public:
        explicit NetworkCoinSpawnerComponentController(NetworkCoinSpawnerComponent& parent);

        void OnActivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
        void OnDeactivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
        
    private:
        AZStd::unordered_map<const AZ::Entity*, AZStd::shared_ptr<AzFramework::EntitySpawnTicket>> m_spawnedCoins;
    };
}
