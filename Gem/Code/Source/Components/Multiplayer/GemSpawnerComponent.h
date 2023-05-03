/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/Component/EntityBus.h>
#include <AzFramework/Spawnable/SpawnableEntitiesInterface.h>
#include <LmbrCentral/Scripting/TagComponentBus.h>
#include <Source/AutoGen/GemSpawnerComponent.AutoComponent.h>

namespace MultiplayerSample
{
    class GemSpawnerComponent
        : public GemSpawnerComponentBase
    {
    public:
        AZ_MULTIPLAYER_COMPONENT(MultiplayerSample::GemSpawnerComponent, s_gemSpawnerComponentConcreteUuid, MultiplayerSample::GemSpawnerComponentBase);

        static void Reflect(AZ::ReflectContext* context);

        void OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating) override {}
        void OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating) override {}
    };


    class GemSpawnerComponentController
        : public GemSpawnerComponentControllerBase
    {
    public:
        explicit GemSpawnerComponentController(GemSpawnerComponent& parent);

        void OnActivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
        void OnDeactivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;

#if AZ_TRAIT_SERVER
        void SpawnGems();
        void SpawnGem(const AZ::Vector3& location, const AZ::Crc32& type);
        void RemoveGem(AzFramework::EntitySpawnTicket::Id gemTicketId);
        void RemoveGems();

        void HandleRPC_SpawnGem(
            AzNetworking::IConnection* invokingConnection, const Multiplayer::NetEntityId& playerEntity, 
            const AZ::Vector3& spawnLocation, const AZStd::string& gemTag) override;
        void HandleRPC_SpawnGemWithValue(
            AzNetworking::IConnection* invokingConnection, const Multiplayer::NetEntityId& playerEntity, 
            const AZ::Vector3& spawnLocation, const AZStd::string& gemTag, const uint16_t& gemValue) override;
#endif

    private:
#if AZ_TRAIT_SERVER
        AZStd::optional<const GemSpawnable> GetGemSpawnable(AZ::Crc32 gemTag) const;
        void SpawnGem(const AZ::Vector3& location, const AzFramework::SpawnableAsset& gemAsset, uint16_t gemValue);
#endif
        AZStd::unordered_map<AzFramework::EntitySpawnTicket::Id, AZStd::shared_ptr<AzFramework::EntitySpawnTicket>> m_spawnedGems;

        struct GemSpawnEntry
        {
            AZ::Crc32 m_tag;
            float m_weight;
            bool m_entityHasTag;

            GemSpawnEntry(AZ::Crc32 tag, float weight, bool entityHasTag)
                : m_tag(tag), m_weight(weight), m_entityHasTag(entityHasTag)
            {
            }
        };

        //! Randomly choose a gem type from the given set of weights for the current round.
        AZ::Crc32 ChooseGemType(AZStd::vector<GemSpawnEntry>& gemSpawnList, const LmbrCentral::Tags& tags);
    };
}
