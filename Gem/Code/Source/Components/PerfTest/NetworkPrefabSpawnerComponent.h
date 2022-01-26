#pragma once

#include <NetworkPrefabSpawnerInterface.h>
#include <AzCore/Component/Component.h>
#include <AzFramework/Spawnable/Spawnable.h>
#include <AzFramework/Spawnable/SpawnableEntitiesInterface.h>

namespace MultiplayerSample
{
    /**
     * \brief Can spawn prefabs using C++ API.
     * Does not keep track of instances. The user should save a copy of the ticket using callbacks in @PrefabCallbacks.
     */
    class NetworkPrefabSpawnerComponent
        : public AZ::Component
        , public NetworkPrefabSpawnerRequestBus::Handler
        , public AZ::Data::AssetBus::MultiHandler
    {
    public:
        AZ_COMPONENT(NetworkPrefabSpawnerComponent, "{7E48961B-7E39-4FBC-95E4-74B712229E9B}", Component);

        static void Reflect(AZ::ReflectContext* reflection);
        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
        {
            provided.push_back(AZ_CRC_CE("NetworkPrefabSpawnerComponent"));
        }

        void Activate() override;
        void Deactivate() override;

        // NetworkPrefabSpawnerRequestBus
        void SpawnPrefab(const AZ::Transform& worldTm, const char* assetPath, PrefabCallbacks callbacks) override;
        void SpawnPrefabAsset(const AZ::Transform& worldTm, const AZ::Data::Asset<AzFramework::Spawnable>& asset, PrefabCallbacks callbacks) override;
        void SpawnDefaultPrefab(const AZ::Transform& worldTm, PrefabCallbacks callbacks) override;

        // AssetBus
        void OnAssetReady(AZ::Data::Asset<AZ::Data::AssetData> asset) override;
        void OnAssetCanceled(AZ::Data::AssetId assetId) override;
        void OnAssetContainerReady(AZ::Data::Asset<AZ::Data::AssetData> asset) override;
        void OnAssetError(AZ::Data::Asset<AZ::Data::AssetData> asset) override;
        void OnAssetMoved(AZ::Data::Asset<AZ::Data::AssetData> asset, void* oldDataPointer) override;
        void OnAssetPreReload(AZ::Data::Asset<AZ::Data::AssetData> asset) override;
        void OnAssetReloadError(AZ::Data::Asset<AZ::Data::AssetData> asset) override;
        void OnAssetReloaded(AZ::Data::Asset<AZ::Data::AssetData> asset) override;
        void OnAssetSaved(AZ::Data::Asset<AZ::Data::AssetData> asset, bool isSuccessful) override;
        void OnAssetUnloaded(const AZ::Data::AssetId assetId, const AZ::Data::AssetType assetType) override;

    private:
        AZ::Data::Asset<AzFramework::Spawnable> m_defaultSpawnableAsset;

        AZ::Data::AssetId GetSpawnableAssetId(const char* assetPath) const;

        struct AssetItem
        {
            AZStd::string m_pathToAsset;
            AZ::Data::Asset<AzFramework::Spawnable> m_spawnableAsset;
        };
        AZStd::unordered_map<AZ::Data::AssetId, AssetItem> m_assetMap;

        struct SpawnRequest
        {
            AZ::Data::AssetId m_assetIdToSpawn;
            AZ::Transform m_whereToSpawn = AZ::Transform::CreateIdentity();
            PrefabCallbacks m_callbacks;
        };

        AZStd::vector<SpawnRequest> m_requests;

        AZStd::vector<AZStd::shared_ptr<AzFramework::EntitySpawnTicket>> m_instanceTickets;
        void CreateInstance(const SpawnRequest& request, const AssetItem* asset);
    };
}
