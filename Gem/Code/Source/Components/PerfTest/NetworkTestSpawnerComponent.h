/*
 * Copyright (c) Contributors to the Open 3D Engine Project
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <random>
#include <AzCore/Component/Component.h>
#include <AzCore/Component/TickBus.h>
#include <AzFramework/Spawnable/SpawnableEntitiesInterface.h>

namespace MultiplayerSample
{
    //! @class NetworkTestSpawnerComponent
    class NetworkTestSpawnerComponent final
        : public AZ::Component
        , public AZ::TickBus::Handler
    {
    public:
        AZ_COMPONENT(MultiplayerSample::NetworkTestSpawnerComponent, "{4F8A554C-99F3-4DDB-8313-3B2FD5F78843}");

        static void Reflect(AZ::ReflectContext* context);
        
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
        {
            required.push_back(AZ_CRC_CE("NetBindService"));
            required.push_back(AZ_CRC_CE("NetworkPrefabSpawnerService"));
        }

        //! AZ::Component overrides.
        //! @{
        void Activate() override;
        void Deactivate() override;
        //! }@

        //! AZ::TickBus overrides.
        //! @{
        void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;
        //! }@

    private:
        bool m_enabled = false;
        int m_maximumLiveCount = 0;
        float m_spawnPeriod = 1.f;

        int m_currentCount = 0;

        float m_accumulatedTime = 0.f;
        float m_sinceLastSpawn = 0.f;

        AZStd::deque<AZStd::shared_ptr<AzFramework::EntitySpawnTicket>> m_spawnedObjects;

        std::uniform_real_distribution<double> m_randomDistribution;
    };
}
