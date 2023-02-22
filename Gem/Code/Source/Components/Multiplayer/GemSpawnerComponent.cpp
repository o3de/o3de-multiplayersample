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
        AZ::EntityBus::MultiHandler::BusDisconnect();
    }

#if AZ_TRAIT_SERVER
    void GemSpawnerComponentController::SpawnGems()
    {
        RemoveGems();

        // Collect all entities marked with a tag to spawn a gem.
        AZ::EBusAggregateResults<AZ::EntityId> aggregator;
        LmbrCentral::TagGlobalRequestBus::EventResult(aggregator, AZ::Crc32(GetGemSpawnTag()),
            &LmbrCentral::TagGlobalRequests::RequestTaggedEntities);

        for (const AZ::EntityId gemSpawnEntity : aggregator.values)
        {
            // Collect the gem tags for this specific entity.
            LmbrCentral::Tags tags;
            LmbrCentral::TagComponentRequestBus::EventResult(tags, gemSpawnEntity,
                &LmbrCentral::TagComponentRequestBus::Events::GetTags);

            // Randomly select a gem type for this entity.
            const AZ::Crc32 type = ChooseGemType(tags);

            AZ::Vector3 position = AZ::Vector3::CreateZero();
            AZ::TransformBus::EventResult(position, gemSpawnEntity, &AZ::TransformBus::Events::GetWorldTranslation);
            SpawnGem(position, type);
        }
    }

    void GemSpawnerComponentController::SpawnGem(const AZ::Vector3& location, const AZ::Crc32& type)
    {
        if (GetParent().GetGemSpawnables().empty())
        {
            return;
        }

        const GemSpawnable* spawnable = nullptr;
        for (const GemSpawnable& gemType : GetParent().GetGemSpawnables())
        {
            if (type == AZ::Crc32(gemType.m_tag.c_str()))
            {
                spawnable = &gemType;
            }
        }
        if (!spawnable)
        {
            return;
        }

        PrefabCallbacks callbacks;
        callbacks.m_onActivateCallback = [this, spawnable](AZStd::shared_ptr<AzFramework::EntitySpawnTicket> ticket,
            AzFramework::SpawnableConstEntityContainerView view)
        {
            for (const AZ::Entity* entity : view)
            {
                if (GemComponent* gem = entity->FindComponent<GemComponent>())
                {
                    if (GemComponentController* gemController = static_cast<GemComponentController*>(gem->GetController()))
                    {
                        gemController->SetRandomPeriodOffset(GetNetworkRandomComponentController()->GetRandomInt() % 1000);
                        gemController->SetGemScoreValue(spawnable->m_scoreValue);
                    }

                    // Save the gem spawn ticket, otherwise the gem will de-spawn
                    m_spawnedGems.emplace(entity->GetId(), AZStd::move(ticket));
                    break;
                }
            }
        };

        GetParent().GetNetworkPrefabSpawnerComponent()->SpawnPrefabAsset(
            AZ::Transform::CreateFromQuaternionAndTranslation(AZ::Quaternion::CreateIdentity(), location),
            spawnable->m_gemAsset, AZStd::move(callbacks));
    }

    void GemSpawnerComponentController::RemoveGems()
    {
        for (const auto& gem : m_spawnedGems)
        {
            const Multiplayer::NetEntityId netEntityId = Multiplayer::GetNetworkEntityManager()->GetNetEntityIdById(gem.first);
            Multiplayer::ConstNetworkEntityHandle netEntityHandle = Multiplayer::GetNetworkEntityManager()->GetEntity(netEntityId);

            if (netEntityHandle.Exists())
            {
                AZ::EntityBus::MultiHandler::BusConnect(gem.first);
                Multiplayer::GetNetworkEntityManager()->MarkForRemoval(netEntityHandle);

                // Move the gem out of the view because it can take a little while before the removal gets applied.
                AZ::TransformBus::Event(gem.first, &AZ::TransformBus::Events::SetWorldTranslation, AZ::Vector3::CreateAxisZ(-1000.f));

                m_queuedForRemovalGems.emplace(gem.first, gem.second);
            }
        }

        m_spawnedGems.clear();
    }
#endif

    void GemSpawnerComponentController::OnEntityDeactivated(const AZ::EntityId& entityId)
    {
        AZ::EntityBus::MultiHandler::BusDisconnect(entityId);
        m_queuedForRemovalGems.erase(entityId);
    }

    AZ::Crc32 GemSpawnerComponentController::ChooseGemType(const LmbrCentral::Tags& tags)
    {
        if (GetSpawnTablesPerRound().empty())
        {
            return {};
        }

        // Get the current round's spawn table, or the last defined round as a fallback.
        const uint16_t round = GetNetworkMatchComponentController()->GetRoundNumber();
        const RoundSpawnTable* table;
        if (round < GetSpawnTablesPerRound().size())
        {
            table = &GetSpawnTablesPerRound()[round];
        }
        else
        {
            table = &GetSpawnTablesPerRound().back();
        }

        // Calculate the total weight of all applicable gem tags.
        float totalWeight = 0.f;
        for (const GemWeightChance& gemWeight : table->m_gemWeights)
        {
            const auto tagIterator = tags.find(AZ::Crc32(gemWeight.m_tag));
            if (tagIterator != tags.end())
            {
                totalWeight += gemWeight.m_weight;
            }
        }
        
        // Create a random float in the range of [0, totalWeight)
        float randomWeight = GetNetworkRandomComponentController()->GetRandomFloat() * totalWeight;

        AZ::Crc32 chosenType(table->m_gemWeights.front().m_tag);
        for (const GemWeightChance& gemWeight : table->m_gemWeights)
        {
            const auto tagIterator = tags.find(AZ::Crc32(gemWeight.m_tag));
            if (tagIterator == tags.end())
            {
                continue;
            }

            // For every acceptable tag, reduce the the weight value until the right gem type is found for the random value.
            // >----------------------\
            //                        |
            // gem1--------gem2-------*--gem3------
            if (randomWeight > gemWeight.m_weight)
            {
                randomWeight -= gemWeight.m_weight;
            }
            else
            {
                chosenType = AZ::Crc32(gemWeight.m_tag);
                break;
            }
        }

        return chosenType;
    }
}
