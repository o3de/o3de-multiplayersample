/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/Component/Component.h>
#include <AzCore/Component/TickBus.h>
#include <AzCore/EBus/Event.h>
#include <DecalBus.h>

#include <Atom/Feature/Decals/DecalFeatureProcessorInterface.h>

namespace MultiplayerSample
{

    class ScriptableDecalComponent
        : public AZ::Component
        , public DecalRequestBus::Handler
        , public AZ::TickBus::Handler
    {
    public:
        AZ_COMPONENT(MultiplayerSample::ScriptableDecalComponent, "{79AEB56C-E886-4A6A-9BAA-0FE5D6D01F78}");

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& services);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& services);

        void Activate() override;
        void Deactivate() override;

    private:

        using DecalHandle = AZ::Render::DecalFeatureProcessorInterface::DecalHandle;

        struct DecalInstance
        {
            SpawnDecalConfig m_config;
            DecalHandle m_handle;
            uint32_t m_despawnMs = 0;
        };

        // DecalRequestBus::Handler...
        void SpawnDecal(const AZ::Transform& worldTm, AZ::Data::AssetId materialAssetId, const SpawnDecalConfig& config) override;

        // TickBus::Handler...
        virtual void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;

        static bool HeapCompare(const DecalInstance& value1, const DecalInstance& value2);
        static uint32_t GetCurrentTimeMs();

        AZ::Render::DecalFeatureProcessorInterface* m_decalFeatureProcessor = nullptr;

        AZStd::vector<DecalInstance> m_decalHeap;
        AZStd::vector<DecalInstance> m_animatingDecals;
    };
}
