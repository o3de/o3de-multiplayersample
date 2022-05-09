/*
 * Copyright (c) Contributors to the Open 3D Engine Project
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <NetworkPrefabSpawnerInterface.h>
#include <AzCore/Component/TransformBus.h>
#include <AzCore/Math/Random.h>
#include <Components/NetworkRandomComponent.h>
#include <Components/PerfTest/NetworkTestSpawnerComponent.h>
#include <LmbrCentral/Shape/ShapeComponentBus.h>
#include <Multiplayer/IMultiplayer.h>
#include <Multiplayer/Components/NetBindComponent.h>
#include <Source/AutoGen/NetworkRandomComponent.AutoComponent.h>
#include "NetworkPrefabSpawnerComponent.h"

namespace MultiplayerSample
{
    NetworkTestSpawnerComponentController::NetworkTestSpawnerComponentController(NetworkTestSpawnerComponent& parent)
        : NetworkTestSpawnerComponentControllerBase(parent)
        , m_tickEvent{ [this] { TickEvent(); }, AZ::Name{ "NetworkTestSpawnerComponent" } }
    {
    }

    void NetworkTestSpawnerComponentController::OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        if (GetParent().GetNetworkPrefabSpawnerComponent())
        {
            m_tickEvent.Enqueue(AZ::TimeMs{ 0 }, true);
        }

        m_currentCount = 0;
        m_accumulatedTime = 0.f;
        m_sinceLastSpawn = 0.f;
    }

    void NetworkTestSpawnerComponentController::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
    }

    void NetworkTestSpawnerComponentController::TickEvent()
    {
        const float deltaTime = static_cast<float>(m_tickEvent.TimeInQueueMs()) / 1000.f;
        m_accumulatedTime += deltaTime;

        if (m_accumulatedTime > 1.0f / aznumeric_cast<float>(GetParent().GetSpawnPerSecond()))
        {
            m_accumulatedTime = 0.f;

            AZ::Vector3 randomPoint = AZ::Vector3::CreateZero();
            // ShapeComponentRequestsBus is designed in such a way that it's very difficult to use direct component interface instead of the EBus
            using ShapeBus = LmbrCentral::ShapeComponentRequestsBus;
            ShapeBus::EventResult(randomPoint, GetParent().GetEntityId(), &ShapeBus::Events::GenerateRandomPointInside,
                AZ::RandomDistributionType::UniformReal);

            AZ::Transform t = GetEntity()->GetTransform()->GetWorldTM();
            if (!randomPoint.IsZero())
            {
                t.SetTranslation(randomPoint);

                // Create a random orientation for fun.
                float randomAngles[3];
                randomAngles[0] = aznumeric_cast<float>(GetNetworkRandomComponentController()->GetRandomUint64() % 180);
                randomAngles[1] = aznumeric_cast<float>(GetNetworkRandomComponentController()->GetRandomUint64() % 180);
                randomAngles[2] = aznumeric_cast<float>(GetNetworkRandomComponentController()->GetRandomUint64() % 180);
                t.SetRotation(AZ::Quaternion::CreateFromEulerAnglesDegrees(AZ::Vector3::CreateFromFloat3(randomAngles)));
            }

            PrefabCallbacks callbacks;
            callbacks.m_onActivateCallback = [this](
                AZStd::shared_ptr<AzFramework::EntitySpawnTicket>&& ticket,
                [[maybe_unused]] AzFramework::SpawnableConstEntityContainerView view)
            {
                m_spawnedObjects.push_back(move(ticket));
            };

            GetParent().GetNetworkPrefabSpawnerComponent()->SpawnDefaultPrefab(t, callbacks);

            m_currentCount++;

            if (m_currentCount >= GetParent().GetMaxLiveCount())
            {
                if (GetParent().GetRespawnEnabled())
                {
                    m_spawnedObjects.pop_front(); // this destroys the prefab instance for this ticket
                    --m_currentCount;
                }
                else
                {
                    m_tickEvent.RemoveFromQueue();
                }
            }
        }
    }
}
