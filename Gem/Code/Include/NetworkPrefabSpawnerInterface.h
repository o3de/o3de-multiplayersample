/*
 * Copyright (c) Contributors to the Open 3D Engine Project
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

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
        AZ_RTTI(NetworkPrefabSpawnerRequests, "{82e5cfb5-6a1a-4bd1-b48d-cd817474d611}");
        virtual ~NetworkPrefabSpawnerRequests() = default;

        /**
         * \brief Spawn a prefab given its asset path at a specified transform.
         * \param worldTm Where to spawn the instance.
         * \param assetPath Path to .spawnable asset to spawn from.
         * \param callbacks Optional structure for pre-activate and post-activate callbacks.
         */
        virtual void SpawnPrefab(const AZ::Transform& worldTm, const char* assetPath, PrefabCallbacks callbacks) = 0;

        /**
         * \brief Spawn a prefab from spawnable asset at a specified transform.
         * \param worldTm Where to spawn the instance.
         * \param asset .spawnable asset to spawn from.
         * \param callbacks Optional structure for pre-activate and post-activate callbacks.
         */
        virtual void SpawnPrefabAsset(const AZ::Transform& worldTm, const AZ::Data::Asset<AzFramework::Spawnable>& asset, PrefabCallbacks callbacks) = 0;

        /**
         * \brief Spawn a prefab instance from spawnable asset assigned in the spawner component. See @NetworkPrefabSpawnerComponent.
         * \param worldTm Where to spawn the instance.
         * \param callbacks Optional structure for pre-activate and post-activate callbacks.
         */
        virtual void SpawnDefaultPrefab(const AZ::Transform& worldTm, PrefabCallbacks callbacks) = 0;
    };

    class NetworkPrefabSpawnerTraits
        : public AZ::ComponentBus
    {
    };

    using NetworkPrefabSpawnerRequestBus = AZ::EBus<NetworkPrefabSpawnerRequests, NetworkPrefabSpawnerTraits>;
    using NetworkPrefabSpawnerInterface = AZ::Interface<NetworkPrefabSpawnerRequests>;
}
