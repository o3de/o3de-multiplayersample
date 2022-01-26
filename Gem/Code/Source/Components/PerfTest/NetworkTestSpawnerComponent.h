/*
 * Copyright (c) Contributors to the Open 3D Engine Project
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/Component/TickBus.h>
#include <AzFramework/Spawnable/SpawnableEntitiesInterface.h>
#include <Source/AutoGen/NetworkTestSpawnerComponent.AutoComponent.h>

namespace MultiplayerSample
{
    class NetworkTestSpawnerComponentController
        : public NetworkTestSpawnerComponentControllerBase
        , public AZ::TickBus::Handler
    {
    public:
        NetworkTestSpawnerComponentController(NetworkTestSpawnerComponent& parent);

        void OnActivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
        void OnDeactivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;

        //! AZ::TickBus overrides.
        //! @{
        void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;
        //! }@

    private:
        int m_currentCount = 0;
        float m_accumulatedTime = 0.f;
        float m_sinceLastSpawn = 0.f;

        AZStd::deque<AZStd::shared_ptr<AzFramework::EntitySpawnTicket>> m_spawnedObjects;
    };
}
