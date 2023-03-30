/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <Components/ScriptableDecalComponent.h>

#include <AzCore/RTTI/BehaviorContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/std/chrono/chrono.h>

#include <Atom/RPI.Public/Scene.h>

namespace MultiplayerSample
{
    void ScriptableDecalComponent::Reflect(AZ::ReflectContext* context)
    {
        SpawnDecalConfig::Reflect(context);

        if (AZ::SerializeContext* serialize = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serialize->Class<MultiplayerSample::ScriptableDecalComponent, AZ::Component>()
                ->Version(0)
                ;

            AZ::EditContext* editContext = serialize->GetEditContext();
            if (editContext)
            {
                editContext->Class<MultiplayerSample::ScriptableDecalComponent>(
                    "Scriptable Decals", "Allows spawning decals directly from script without prefabs.")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Category, "Graphics")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZStd::vector<AZ::Crc32>({ AZ_CRC_CE("Level") }))
                    ;
            }
        }

        AZ::BehaviorContext* behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context);
        if (behaviorContext)
        {
            behaviorContext->EBus<DecalRequestBus>("RequestBus")
                ->Attribute(AZ::Script::Attributes::Scope, AZ::Script::Attributes::ScopeFlags::Common)
                ->Attribute(AZ::Script::Attributes::Category, "Rendering")
                ->Attribute(AZ::Script::Attributes::Module, "rendering")
                ->Attribute(AZ::Script::Attributes::Scope, AZ::Script::Attributes::ScopeFlags::Common)
                ->Event(
                    "SpawnDecal",
                    &DecalRequestBus::Events::SpawnDecal)
                ;
        }
    }

    void ScriptableDecalComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& services)
    {
        services.push_back(AZ_CRC_CE("ScriptableDecalService"));
    }

    void ScriptableDecalComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& services)
    {
        services.push_back(AZ_CRC_CE("ScriptableDecalService"));
    }

    void ScriptableDecalComponent::Activate()
    {
        AZ_Printf("ScriptableDecalComponent", "Activate");
        m_decalFeatureProcessor = AZ::RPI::Scene::GetFeatureProcessorForEntity<AZ::Render::DecalFeatureProcessorInterface>(GetEntityId());
        if (m_decalFeatureProcessor)
        {
            AZ::RPI::Scene* scene = m_decalFeatureProcessor->GetParentScene();
            DecalRequestBus::Handler::BusConnect(scene->GetId());
            AZ::TickBus::Handler::BusConnect();
        }
    }

    void ScriptableDecalComponent::Deactivate()
    {
        AZ::TickBus::Handler::BusDisconnect();
        DecalRequestBus::Handler::BusDisconnect();
        m_decalFeatureProcessor = nullptr;
    }

    void ScriptableDecalComponent::SpawnDecal(const AZ::Transform& worldTm, AZ::Data::AssetId materialAssetId, const SpawnDecalConfig& config)
    {
        DecalHandle handle = m_decalFeatureProcessor->AcquireDecal();

        AZ::Transform transform = worldTm * AZ::Transform::CreateUniformScale(config.m_scale);

        m_decalFeatureProcessor->SetDecalTransform(handle, transform);
        m_decalFeatureProcessor->SetDecalMaterial(handle, materialAssetId);
        m_decalFeatureProcessor->SetDecalOpacity(handle, config.m_opacity);
        m_decalFeatureProcessor->SetDecalAttenuationAngle(handle, config.m_attenutationAngle);
        m_decalFeatureProcessor->SetDecalSortKey(handle, config.m_sortKey);

        uint32_t currentTimeMs = GetCurrentTimeMs();

        if (config.m_lifeTime > 0.0)
        {
            uint32_t lifetimeMs = static_cast<uint32_t>(config.m_lifeTime * 1000.0f);
            uint32_t despawnTimeMs = currentTimeMs + lifetimeMs;

            m_decalHeap.push_back({ config , handle, despawnTimeMs });
            AZStd::push_heap(m_decalHeap.begin(), m_decalHeap.end(), HeapCompare);
        }
        else
        {
            m_animatingDecals.push_back({ config , handle, currentTimeMs });
        }
    }

    void ScriptableDecalComponent::OnTick([[maybe_unused]] float deltaTime, AZ::ScriptTimePoint time)
    {
        uint32_t currentTimeMs = static_cast<uint32_t>(time.GetMilliseconds());

        // Check to see if any decals need to despawn
        while (!m_decalHeap.empty() && m_decalHeap.front().m_despawnMs < currentTimeMs)
        {
            AZStd::pop_heap(m_decalHeap.begin(), m_decalHeap.end(), HeapCompare);
            m_animatingDecals.push_back(m_decalHeap.back());
            m_decalHeap.pop_back();
        }

        // Animate despawning decals, remove those that are expired.
        for (size_t i = 0; i < m_animatingDecals.size();)
        {
            DecalInstance& decalInstance = m_animatingDecals.at(i);

            float currentFadeTimeMs = static_cast<float>(currentTimeMs - decalInstance.m_despawnMs);
            float totalFadeTimeMs = decalInstance.m_config.m_fadeTime * 1000.0f;
            if (currentFadeTimeMs > totalFadeTimeMs)
            {
                // Despawn the decal, it's done animating;
                m_decalFeatureProcessor->ReleaseDecal(decalInstance.m_handle);

                // Replace this instance with the one on the back
                decalInstance = m_animatingDecals.back();
                m_animatingDecals.pop_back();

                // Don't increment, next iteration needs to process the item just moved to this spot.
            }
            else
            {
                float opacity = 1.0f - (currentFadeTimeMs / totalFadeTimeMs);
                m_decalFeatureProcessor->SetDecalOpacity(decalInstance.m_handle, opacity);
                ++i;
            }
        }
    }

    bool ScriptableDecalComponent::HeapCompare(const DecalInstance& value1, const DecalInstance& value2)
    {
        return value1.m_despawnMs < value2.m_despawnMs;
    }

    uint32_t ScriptableDecalComponent::GetCurrentTimeMs()
    {
        auto now = AZStd::chrono::steady_clock::now().time_since_epoch();
        auto nowMs = AZStd::chrono::duration_cast<AZStd::chrono::milliseconds>(now).count();
        return static_cast<uint32_t>(nowMs);
    }
}