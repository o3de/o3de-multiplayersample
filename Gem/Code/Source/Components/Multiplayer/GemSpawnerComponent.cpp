/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <MultiplayerSampleTypes.h>
#include <AzCore/Component/TransformBus.h>
#include <AzCore/Serialization/EditContext.h>
#include <Components/NetworkMatchComponent.h>
#include <LmbrCentral/Scripting/TagComponentBus.h>
#include <LmbrCentral/Shape/ShapeComponentBus.h>
#include <Source/Components/NetworkRandomComponent.h>
#include <Source/Components/Multiplayer/GemComponent.h>
#include <Source/Components/Multiplayer/GemSpawnerComponent.h>
#include <Source/Components/PerfTest/NetworkPrefabSpawnerComponent.h>

namespace MultiplayerSample
{
    void GemSpawnable::Reflect(AZ::ReflectContext* context)
    {
        AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context);
        if (serializeContext)
        {
            serializeContext->Class<GemSpawnable>()
                ->Version(2)
                ->Field("Tag", &GemSpawnable::m_tag)
                ->Field("Asset", &GemSpawnable::m_gemAsset)
                ->Field("Score", &GemSpawnable::m_scoreValue)
                ;

            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                editContext->Class<GemSpawnable>("GemSpawnable", "Defines a gem type with an asset and a tag name.")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &GemSpawnable::m_tag, "Tag", "Assigned tag for this gem type")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &GemSpawnable::m_gemAsset, "Asset", "Spawnable for the gem")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &GemSpawnable::m_scoreValue, "Score", "Gem's value")
                    ;
            }
        }
    }

    void GemWeightChance::Reflect(AZ::ReflectContext* context)
    {
        AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context);
        if (serializeContext)
        {
            serializeContext->Class<GemWeightChance>()
                ->Version(1)
                ->Field("Gem Tag Type", &GemWeightChance::m_tag)
                ->Field("Gem Weight", &GemWeightChance::m_weight)
                ;

            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                editContext->Class<GemWeightChance>("GemWeightChance", "Defines a weighted chance for a gem type to spawn in a given round.")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &GemWeightChance::m_tag, "Gem Tag Type", "Assigned tag for this gem type")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &GemWeightChance::m_weight, "Gem Weight", "Weight value in randomly choosing between the gems")
                    ;
            }
        }
    }

    void RoundSpawnTable::Reflect(AZ::ReflectContext* context)
    {
        AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context);
        if (serializeContext)
        {
            serializeContext->Class<RoundSpawnTable>()
                ->Version(1)
                ->Field("Gem Weights", &RoundSpawnTable::m_gemWeights)
                ;

            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                editContext->Class<RoundSpawnTable>("RoundSpawnTable", "Defines chances for gem types to spawn in a given round.")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &RoundSpawnTable::m_gemWeights, "Gem Weights", "Gem weights for a given round")
                    ;
            }
        }
    }

    void GemSpawnerComponent::Reflect(AZ::ReflectContext* context)
    {
        AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context);
        if (serializeContext)
        {
            serializeContext->Class<GemSpawnerComponent, GemSpawnerComponentBase>()
                ->Version(1);
        }
        GemSpawnerComponentBase::Reflect(context);
    }


    GemSpawnerComponentController::GemSpawnerComponentController(GemSpawnerComponent& parent)
        : GemSpawnerComponentControllerBase(parent)
    {
    }

    void GemSpawnerComponentController::OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
    }

    void GemSpawnerComponentController::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
#if AZ_TRAIT_SERVER
        RemoveGems();
#endif
    }

#if AZ_TRAIT_SERVER
    void GemSpawnerComponentController::SpawnGems()
    {
        RemoveGems();

        // Collect all entities marked with a tag to spawn a gem.
        AZ::EBusAggregateResults<AZ::EntityId> aggregator;
        LmbrCentral::TagGlobalRequestBus::EventResult(aggregator, AZ::Crc32(GetGemSpawnTag()),
            &LmbrCentral::TagGlobalRequests::RequestTaggedEntities);

        // If there aren't any spawn tables, don't spawn anything.
        if (GetSpawnTablesPerRound().empty())
        {
            return;
        }

        // Get the current round's spawn table, or the last defined round as a fallback.
        const uint16_t round = GetNetworkMatchComponentController()->GetRoundNumber();
        const RoundSpawnTable& table = 
            GetSpawnTablesPerRound()[AZStd::min(round, aznumeric_cast<uint16_t>(GetSpawnTablesPerRound().size() - 1))];

        // Move the table data into a working list where we've done a one-time conversion of tags into CRCs and we can temporarily 
        // store which tags exist on the entity.
        AZStd::vector<GemSpawnEntry> gemSpawnList;
        gemSpawnList.reserve(table.m_gemWeights.size());
        for (const GemWeightChance& gemWeight : table.m_gemWeights)
        {
            gemSpawnList.emplace_back(AZ::Crc32(gemWeight.m_tag), gemWeight.m_weight, false);
        }

        for (const AZ::EntityId gemSpawnEntity : aggregator.values)
        {
            // Collect the gem tags for this specific entity.
            LmbrCentral::Tags tags;
            LmbrCentral::TagComponentRequestBus::EventResult(tags, gemSpawnEntity,
                &LmbrCentral::TagComponentRequestBus::Events::GetTags);

            // Randomly select a gem type for this entity.
            const AZ::Crc32 type = ChooseGemType(gemSpawnList, tags);

            // If this entity has a valid gem type, spawn it.
            if (type != AZ::Crc32(0))
            {
                AZ::Vector3 position = AZ::Vector3::CreateZero();
                AZ::TransformBus::EventResult(position, gemSpawnEntity, &AZ::TransformBus::Events::GetWorldTranslation);
                SpawnGem(position, type);
            }
        }
    }

    void GemSpawnerComponentController::HandleRPC_SpawnGem(
        [[maybe_unused]] AzNetworking::IConnection* invokingConnection, 
        [[maybe_unused]] const Multiplayer::NetEntityId& playerEntity, const AZ::Vector3& spawnLocation, const AZStd::string& gemTag)
    {
        SpawnGem(spawnLocation, AZ::Crc32(gemTag));
    }

    void GemSpawnerComponentController::HandleRPC_SpawnGemWithValue(
        [[maybe_unused]] AzNetworking::IConnection* invokingConnection,
        [[maybe_unused]] const Multiplayer::NetEntityId& playerEntity, 
        const AZ::Vector3& spawnLocation, const AZStd::string& gemTag, const uint16_t& gemValue)
    {
        
        if (auto gemEntry = GetGemSpawnable(AZ::Crc32(gemTag)); gemEntry)
        { 
            // Spawn the gem with the max value between what's requested and what's in the gem table.
            uint16_t value = AZStd::max(gemEntry->m_scoreValue, gemValue);
            SpawnGem(spawnLocation, gemEntry->m_gemAsset, value);
        }
    }

    AZStd::optional<const GemSpawnable> GemSpawnerComponentController::GetGemSpawnable(AZ::Crc32 gemTag) const
    {
        for (const GemSpawnable gemType : GetParent().GetGemSpawnables())
        {
            if (gemTag == AZ::Crc32(gemType.m_tag.c_str()))
            {
                return gemType;
            }
        }

        return {};
    }

    void GemSpawnerComponentController::SpawnGem(const AZ::Vector3& location, const AZ::Crc32& type)
    {
        if (auto gemEntry = GetGemSpawnable(type); gemEntry)
        {
            SpawnGem(location, gemEntry->m_gemAsset, gemEntry->m_scoreValue);
        }
    }

    void GemSpawnerComponentController::SpawnGem(const AZ::Vector3& location, const AzFramework::SpawnableAsset& gemAsset, uint16_t gemValue)
    {
        // Don't spawn gems with 0 value.
        if (gemValue == 0)
        {
            return;
        }

        PrefabCallbacks callbacks;
        callbacks.m_onActivateCallback = [this, gemValue](AZStd::shared_ptr<AzFramework::EntitySpawnTicket> ticket,
            AzFramework::SpawnableConstEntityContainerView view)
        {
            if (view.empty())
            {
                return;
            }

            const auto ticketId = ticket->GetId();

            for (const AZ::Entity* entity : view)
            {
                if (GemComponent* gem = entity->FindComponent<GemComponent>())
                {
                    if (GemComponentController* gemController = static_cast<GemComponentController*>(gem->GetController()))
                    {
                        gemController->SetRandomPeriodOffset(GetNetworkRandomComponentController()->GetRandomInt() % 1000);
                        gemController->SetGemScoreValue(gemValue);
                        gemController->SetGemSpawnerController(this);
                    }
                }
            }

            // Save the gem spawn ticket, otherwise the gem will immediately despawn due to the ticket's destruction.
            // Also track the root entity id so that we can move the gem out of sight while waiting for it to despawn when removing gems.
            m_spawnedGems.insert(AZStd::make_pair(ticketId, AZStd::move(ticket)));
        };

        GetParent().GetNetworkPrefabSpawnerComponent()->SpawnPrefabAsset(
            AZ::Transform::CreateFromQuaternionAndTranslation(AZ::Quaternion::CreateIdentity(), location),
            gemAsset, AZStd::move(callbacks));
    }

    void GemSpawnerComponentController::RemoveGems()
    {
        for (const auto& pair : m_spawnedGems)
        {
            // Destroy all the entities for each gem.
            AzFramework::SpawnableEntitiesInterface::Get()->DespawnAllEntities(*pair.second);
        }

        m_spawnedGems.clear();
    }
    
    void GemSpawnerComponentController::RemoveGem(AzFramework::EntitySpawnTicket::Id gemTicketId)
    {
        const auto gemIterator = m_spawnedGems.find(gemTicketId);        

        if (gemIterator != m_spawnedGems.end())
        {
            AzFramework::SpawnableEntitiesInterface::Get()->DespawnAllEntities(*gemIterator->second);
            m_spawnedGems.erase(gemIterator);
        }
    }
#endif

    AZ::Crc32 GemSpawnerComponentController::ChooseGemType(AZStd::vector<GemSpawnEntry>& gemSpawnList, const LmbrCentral::Tags& tags)
    {
        // Calculate the total weight of all applicable gem tags.
        float totalWeight = 0.f;
        for (GemSpawnEntry& entry : gemSpawnList)
        {
            const auto tagIterator = tags.find(entry.m_tag);
            entry.m_entityHasTag = tagIterator != tags.end();
            if (tagIterator != tags.end())
            {
                totalWeight += entry.m_weight;
            }
        }
        
        // Create a random float in the range of [0, totalWeight)
        float randomWeight = GetNetworkRandomComponentController()->GetRandomFloat() * totalWeight;

        AZ::Crc32 chosenType;
        for (GemSpawnEntry& entry : gemSpawnList)
        {
            if (entry.m_entityHasTag)
            {
                // For every acceptable tag, reduce the the weight value until the right gem type is found for the random value.
                // >----------------------\
                //                        |
                // gem1--------gem2-------*--gem3------
                if (randomWeight > entry.m_weight)
                {
                    randomWeight -= entry.m_weight;
                }
                else
                {
                    chosenType = entry.m_tag;
                    break;
                }

            }
        }

        return chosenType;
    }
}
