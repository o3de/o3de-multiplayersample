
#include "NetworkPrefabSpawnerComponent.h"

#include <AzCore/Asset/AssetManagerBus.h>
#include <AzCore/Asset/AssetSerializer.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/std/smart_ptr/make_shared.h>
#include <AzFramework/Components/TransformComponent.h>
#include <AzFramework/Spawnable/SpawnableEntitiesInterface.h>

namespace MultiplayerSample
{
    void NetworkPrefabSpawnerComponent::Reflect(AZ::ReflectContext* reflection)
    {
        if (const auto serializationContext = azrtti_cast<AZ::SerializeContext*>(reflection))
        {
            serializationContext->Class<NetworkPrefabSpawnerComponent, Component>()
                ->Field("Default Prefab", &NetworkPrefabSpawnerComponent::m_defaultSpawnableAsset)
                ->Version(1);

            if (const auto editContext = serializationContext->GetEditContext())
            {
                editContext->Class<NetworkPrefabSpawnerComponent>("Network Prefab Spawner",
                    "Handles spawning of prefabs")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Category, "MultiplayerSample")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC_CE("Game"))
                    ->DataElement(nullptr, &NetworkPrefabSpawnerComponent::m_defaultSpawnableAsset, "Default Prefab", "Default prefab to spawn upon request.")
                    ;
            }
        }
    }

    void NetworkPrefabSpawnerComponent::Activate()
    {
        if (NetworkPrefabSpawnerInterface::Get() == nullptr)
        {
            NetworkPrefabSpawnerInterface::Register(this);
        }

        NetworkPrefabSpawnerRequestBus::Handler::BusConnect(GetEntityId());

        // preload
        if (m_defaultSpawnableAsset.GetId().IsValid())
        {
            m_defaultSpawnableAsset.QueueLoad();
        }
    }

    void NetworkPrefabSpawnerComponent::Deactivate()
    {
        if (NetworkPrefabSpawnerInterface::Get() == this)
        {
            NetworkPrefabSpawnerInterface::Unregister(this);
        }

        NetworkPrefabSpawnerRequestBus::Handler::BusDisconnect();
        AZ::Data::AssetBus::MultiHandler::BusDisconnect();
    }

    void NetworkPrefabSpawnerComponent::SpawnDefaultPrefab(const AZ::Transform& worldTm, PrefabCallbacks callbacks)
    {
        AssetItem newAsset;
        newAsset.m_pathToAsset = m_defaultSpawnableAsset.GetHint().c_str();
        newAsset.m_spawnableAsset = m_defaultSpawnableAsset;
        AZ::Data::AssetBus::MultiHandler::BusConnect(m_defaultSpawnableAsset.GetId());

        if (newAsset.m_spawnableAsset.IsReady() == false)
        {
            newAsset.m_spawnableAsset.QueueLoad();
        }

        m_assetMap.emplace(newAsset.m_spawnableAsset.GetId(), newAsset);

        const SpawnRequest request{ newAsset.m_spawnableAsset.GetId(), worldTm, AZStd::move(callbacks) };
        if (newAsset.m_spawnableAsset.IsReady())
        {
            CreateInstance(request, &newAsset);
        }
        else
        {
            m_requests.push_back(request);
        }
    }

    void NetworkPrefabSpawnerComponent::SpawnPrefab(const AZ::Transform& worldTm, const char* assetPath, PrefabCallbacks callbacks)
    {
        const AZ::Data::AssetId assetId = GetSpawnableAssetId(assetPath);

        const SpawnRequest request{ assetId, worldTm, AZStd::move(callbacks) };
        auto foundAsset = m_assetMap.find(assetId);
        if (foundAsset != m_assetMap.end())
        {
            if (foundAsset->second.m_spawnableAsset.IsReady())
            {
                CreateInstance(request, &foundAsset->second);
            }
            else
            {
                m_requests.push_back(request);
            }
        }
        else
        {
            AssetItem newAsset;
            newAsset.m_pathToAsset = assetPath;
            newAsset.m_spawnableAsset.Create(assetId, false);
            AZ::Data::AssetBus::MultiHandler::BusConnect(assetId);
            newAsset.m_spawnableAsset.QueueLoad();
            m_assetMap.emplace(assetId, newAsset);

            if (newAsset.m_spawnableAsset.IsReady())
            {
                CreateInstance(request, &newAsset);
            }
            else
            {
                m_requests.push_back(request);
            }
        }
    }

    void NetworkPrefabSpawnerComponent::SpawnPrefabAsset(const AZ::Transform& worldTm,
        const AZ::Data::Asset<AzFramework::Spawnable>& asset, PrefabCallbacks callbacks)
    {
        AssetItem newAsset;
        newAsset.m_pathToAsset = asset.GetHint().c_str();
        newAsset.m_spawnableAsset = asset;
        AZ::Data::AssetBus::MultiHandler::BusConnect(asset.GetId());

        if (newAsset.m_spawnableAsset.IsReady() == false)
        {
            newAsset.m_spawnableAsset.QueueLoad();
        }

        m_assetMap.emplace(newAsset.m_spawnableAsset.GetId(), newAsset);

        const SpawnRequest request{ newAsset.m_spawnableAsset.GetId(), worldTm, AZStd::move(callbacks) };
        if (newAsset.m_spawnableAsset.IsReady())
        {
            CreateInstance(request, &newAsset);
        }
        else
        {
            m_requests.push_back(request);
        }
    }

    AZ::Data::AssetId NetworkPrefabSpawnerComponent::GetSpawnableAssetId(const char* assetPath) const
    {
        if (assetPath)
        {
            AZ::Data::AssetId assetId;
            AZ::Data::AssetCatalogRequestBus::BroadcastResult(
                assetId, &AZ::Data::AssetCatalogRequestBus::Events::GetAssetIdByPath, assetPath,
                AZ::Data::s_invalidAssetType, false);
            if (assetId.IsValid())
            {
                return assetId;
            }
        }

        return {};
    }

    void NetworkPrefabSpawnerComponent::CreateInstance(const SpawnRequest& request, const AssetItem* asset)
    {
        AZ::Transform world = request.m_whereToSpawn;

        if (asset)
        {
            auto ticket = AZStd::make_shared<AzFramework::EntitySpawnTicket>(asset->m_spawnableAsset);

            auto preSpawnCallback = [world, request, ticket]([[maybe_unused]] AzFramework::EntitySpawnTicket::Id ticketId, AzFramework::SpawnableEntityContainerView view)
            {
                const AZ::Entity* rootEntity = *view.begin();
                if (AzFramework::TransformComponent* entityTransform = rootEntity->FindComponent<AzFramework::TransformComponent>())
                {
                    entityTransform->SetWorldTM(world);
                }

                if (request.m_callbacks.m_beforeActivateCallback)
                {
                    request.m_callbacks.m_beforeActivateCallback(ticket, view);
                }
            };

            auto onSpawnedCallback = [request, ticket]([[maybe_unused]] AzFramework::EntitySpawnTicket::Id ticketId, AzFramework::SpawnableConstEntityContainerView view)
            {
                if (request.m_callbacks.m_onActivateCallback)
                {
                    request.m_callbacks.m_onActivateCallback(ticket, view);
                }
            };

            if (ticket->IsValid())
            {
                AzFramework::SpawnAllEntitiesOptionalArgs optionalArgs;
                optionalArgs.m_preInsertionCallback = AZStd::move(preSpawnCallback);
                optionalArgs.m_completionCallback = AZStd::move(onSpawnedCallback);
                AzFramework::SpawnableEntitiesInterface::Get()->SpawnAllEntities(*ticket, AZStd::move(optionalArgs));
            }
            else
            {
                AZ_Assert(ticket->IsValid(), "Unable to instantiate spawnable asset");
            }
        }
        else
        {
            AZ_Assert(asset, "AssetMap didn't contain the asset id for prefab spawning");
        }
    }

    void NetworkPrefabSpawnerComponent::OnAssetReady(AZ::Data::Asset<AZ::Data::AssetData> asset)
    {
        const AZ::Data::AssetId assetId = asset.GetId();
        AZ::Data::AssetBus::MultiHandler::BusDisconnect(assetId);

        const auto foundAsset = m_assetMap.find(assetId);
        if (foundAsset != m_assetMap.end())
        {
            for (auto requestIterator = m_requests.begin(); requestIterator < m_requests.end(); /*iterating inside the loop body*/)
            {
                const SpawnRequest& request = *requestIterator;

                if (request.m_assetIdToSpawn == assetId)
                {
                    CreateInstance(request, &foundAsset->second);
                    requestIterator = m_requests.erase(requestIterator);
                }
                else
                {
                    ++requestIterator;
                }
            }
        }
    }
}
