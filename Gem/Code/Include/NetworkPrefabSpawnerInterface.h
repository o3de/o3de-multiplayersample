#pragma once

#include <AzCore/Math/Transform.h>
#include <AzFramework/Spawnable/SpawnableEntitiesInterface.h>

namespace MultiplayerSample
{
    using PrefabSpawnCallbackBeforeActivation = AZStd::function<void(
        AZStd::shared_ptr<AzFramework::EntitySpawnTicket>,
        AzFramework::SpawnableEntityContainerView)>;

    using PrefabSpawnCallback = AZStd::function<void(
        AZStd::shared_ptr<AzFramework::EntitySpawnTicket>,
        AzFramework::SpawnableConstEntityContainerView)>;

    struct PrefabCallbacks
    {
        PrefabSpawnCallbackBeforeActivation m_beforeActivateCallback;
        PrefabSpawnCallback m_onActivateCallback;
    };

    class NetworkPrefabSpawnerRequests
    {
    public:
        AZ_RTTI(RecastO3DEGemRequests, "{82e5cfb5-6a1a-4bd1-b48d-cd817474d611}");
        virtual ~NetworkPrefabSpawnerRequests() = default;

        virtual void SpawnPrefab(const AZ::Transform& worldTm, const char* assetPath, PrefabCallbacks callbacks) = 0;
        virtual void SpawnPrefabAsset(const AZ::Transform& worldTm, const AZ::Data::Asset<AzFramework::Spawnable>& asset, PrefabCallbacks callbacks) = 0;
        virtual void SpawnDefaultPrefab(const AZ::Transform& worldTm, PrefabCallbacks callbacks) = 0;
    };

    class NetworkPrefabSpawnerTraits
        : public AZ::ComponentBus
    {
    };

    using NetworkPrefabSpawnerRequestBus = AZ::EBus<NetworkPrefabSpawnerRequests, NetworkPrefabSpawnerTraits>;
    using NetworkPrefabSpawnerInterface = AZ::Interface<NetworkPrefabSpawnerRequests>;
}
