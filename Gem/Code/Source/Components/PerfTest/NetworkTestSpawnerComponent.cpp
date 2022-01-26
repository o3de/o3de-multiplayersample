/*
 * Copyright (c) Contributors to the Open 3D Engine Project
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AzCore/Component/TransformBus.h>
#include <AzCore/Math/Random.h>
#include <AzCore/Serialization/EditContext.h>
#include <Components/PerfTest/NetworkTestSpawnerComponent.h>
#include <LmbrCentral/Shape/ShapeComponentBus.h>
#include <Multiplayer/IMultiplayer.h>
#include <Multiplayer/Components/NetBindComponent.h>

#include "NetworkPrefabSpawnerComponent.h"

#pragma optimize("", off)

namespace LmbrCentral
{
    class BoxShapeComponent;
}

namespace MultiplayerSample
{
    void NetworkTestSpawnerComponent::Reflect(AZ::ReflectContext* context)
    {
        AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context);
        if (serializeContext)
        {
            serializeContext->Class<NetworkTestSpawnerComponent, AZ::Component>()
                ->Field("Enabled", &NetworkTestSpawnerComponent::m_enabled)
                ->Field("Max Objects", &NetworkTestSpawnerComponent::m_maximumLiveCount)
                ->Field("Spawn Period", &NetworkTestSpawnerComponent::m_spawnPeriod)
                ->Version(1);

            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                using namespace AZ::Edit;
                editContext->Class<NetworkTestSpawnerComponent>("Network Prefab Spawn Tester",
                    "Various helpful test tools and behaviors to test multiplayer logic and performance.")
                    ->ClassElement(ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Category, "MultiplayerSample")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC_CE("Game"))
                    ->DataElement(nullptr, &NetworkTestSpawnerComponent::m_enabled, "Enabled", "Enables spawning of test prefabs")
                    ->DataElement(nullptr, &NetworkTestSpawnerComponent::m_spawnPeriod, "Spawn Period", "How often to spawn new prefab instance, in seconds")
                        ->Attribute(AZ::Edit::Attributes::Suffix, " seconds")
                    ->DataElement(nullptr, &NetworkTestSpawnerComponent::m_maximumLiveCount, "Max Objects", 
                        "Maximum objects to keep alive, will delete older objects when the count goes above this value.")
                    ;
            }
        }
    }

    void NetworkTestSpawnerComponent::Activate()
    {
        m_randomDistribution = std::uniform_real_distribution<double>(-1000.f, 1000.f);

        if (const Multiplayer::NetBindComponent* netBindComponent = GetEntity()->FindComponent<Multiplayer::NetBindComponent>())
        {
            if (netBindComponent->IsNetEntityRoleAuthority())
            {
                AZ::TickBus::Handler::BusConnect();
                
                m_currentCount = 0;
                m_accumulatedTime = 0.f;
                m_sinceLastSpawn = 0.f;
            }
        }
    }

    void NetworkTestSpawnerComponent::Deactivate()
    {
        AZ::TickBus::Handler::BusDisconnect();
    }

    void NetworkTestSpawnerComponent::OnTick(float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time)
    {
        m_accumulatedTime += deltaTime;

        if (m_accumulatedTime > m_spawnPeriod)
        {
            m_accumulatedTime = 0.f;

            if (NetworkPrefabSpawnerComponent* spawner = GetEntity()->FindComponent<NetworkPrefabSpawnerComponent>())
            {
                AZ::Transform t = GetEntity()->GetTransform()->GetWorldTM();

                AZ::Vector3 randomPoint = AZ::Vector3::CreateZero();
                using ShapeBus = LmbrCentral::ShapeComponentRequestsBus;
                ShapeBus::EventResult(randomPoint, GetEntityId(), &ShapeBus::Events::GenerateRandomPointInside, AZ::RandomDistributionType::UniformReal);
                if (!randomPoint.IsZero())
                {
                    t.SetTranslation(randomPoint);
                }

                PrefabCallbacks callbacks;
                callbacks.m_onActivateCallback = [this](
                    AZStd::shared_ptr<AzFramework::EntitySpawnTicket>&& ticket,
                    [[maybe_unused]] AzFramework::SpawnableConstEntityContainerView view)
                {
                    m_spawnedObjects.push_back(move(ticket));
                };

                spawner->SpawnDefaultPrefab(t, callbacks);
            }

            m_currentCount++;

            if (m_currentCount > m_maximumLiveCount)
            {
                m_spawnedObjects.pop_front();
                m_currentCount--;
            }
        }
    }
}
