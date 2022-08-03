/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <LmbrCentral/Shape/ShapeComponentBus.h>
#include <Source/Components/NetworkCoinSpawnerComponent.h>
#include <Source/Components/NetworkRandomComponent.h>
#include <Source/Components/PerfTest/NetworkPrefabSpawnerComponent.h>

namespace MultiplayerSample
{
    void NetworkCoinSpawnerComponent::Reflect(AZ::ReflectContext* context)
    {
        AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context);
        if (serializeContext)
        {
            serializeContext->Class<NetworkCoinSpawnerComponent, NetworkCoinSpawnerComponentBase>()
                ->Version(1);
        }
        NetworkCoinSpawnerComponentBase::Reflect(context);
    }


    NetworkCoinSpawnerComponentController::NetworkCoinSpawnerComponentController(NetworkCoinSpawnerComponent& parent)
        : NetworkCoinSpawnerComponentControllerBase(parent)
    {
    }

    void NetworkCoinSpawnerComponentController::OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        // Spawn coins inside the volume.
        AZ::Aabb areaBounds = AZ::Aabb::CreateNull();
        LmbrCentral::ShapeComponentRequestsBus::EventResult(areaBounds, GetEntityId(),
            &LmbrCentral::ShapeComponentRequestsBus::Events::GetEncompassingAabb);

        const int sideX = aznumeric_cast<int>(areaBounds.GetXExtent());
        const int sideY = aznumeric_cast<int>(areaBounds.GetYExtent());
        
        for (int coinIndex = 0; coinIndex < GetCoinCountToSpawn(); ++coinIndex)
        {
            const float x = aznumeric_cast<float>(GetNetworkRandomComponentController()->GetRandomInt() % sideX);
            const float y = aznumeric_cast<float>(GetNetworkRandomComponentController()->GetRandomInt() % sideY);
            SpawnCoin(areaBounds.GetMin() + AZ::Vector3(x, y, 0.f));
        }
    }

    void NetworkCoinSpawnerComponentController::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
    }

    void NetworkCoinSpawnerComponentController::SpawnCoin(const AZ::Vector3& location)
    {
        PrefabCallbacks callbacks;
        callbacks.m_onActivateCallback = [this](AZStd::shared_ptr<AzFramework::EntitySpawnTicket> ticket,
            AzFramework::SpawnableConstEntityContainerView view)
        {
            for (const AZ::Entity* entity : view)
            {
                if (NetworkCoinComponent* coin = entity->FindComponent<NetworkCoinComponent>())
                {
                    static_cast<NetworkCoinComponentController*>(coin->GetController())->SetRandomPeriodOffset(
                        GetNetworkRandomComponentController()->GetRandomInt() % 1000);

                    // Save the coin spawn ticket, otherwise the coin will de-spawn
                    m_spawnedCoins.emplace(entity, AZStd::move(ticket));
                    break;
                }
            }
        };

        GetParent().GetNetworkPrefabSpawnerComponent()->SpawnPrefabAsset(
            AZ::Transform::CreateFromQuaternionAndTranslation(AZ::Quaternion::CreateIdentity(), location),
            GetParent().GetCoinSpawnable(), AZStd::move(callbacks));
    }
}
