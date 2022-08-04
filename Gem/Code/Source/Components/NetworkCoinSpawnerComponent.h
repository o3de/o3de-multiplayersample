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
    class NetworkCoinSpawnerComponent
        : public NetworkCoinSpawnerComponentBase
    {
    public:
        AZ_MULTIPLAYER_COMPONENT(MultiplayerSample::NetworkCoinSpawnerComponent, s_networkCoinSpawnerComponentConcreteUuid, MultiplayerSample::NetworkCoinSpawnerComponentBase);

        static void Reflect(AZ::ReflectContext* context);

        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
        {
            required.push_back(AZ_CRC_CE("AxisAlignedBoxShapeService"));
            NetworkCoinSpawnerComponentBase::GetRequiredServices(required);
        }
        
        void OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating) override {}
        void OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating) override {}
    };


    class NetworkCoinSpawnerComponentController
        : public NetworkCoinSpawnerComponentControllerBase
    {
    public:
        explicit NetworkCoinSpawnerComponentController(NetworkCoinSpawnerComponent& parent);

        void OnActivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
        void OnDeactivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
        
    private:
        AZStd::unordered_map<const AZ::Entity*, AZStd::shared_ptr<AzFramework::EntitySpawnTicket>> m_spawnedCoins;

        void SpawnCoin(const AZ::Vector3& location);
    };
}
