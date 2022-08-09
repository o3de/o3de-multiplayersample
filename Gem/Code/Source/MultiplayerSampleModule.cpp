/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AzCore/Memory/SystemAllocator.h>
#include <AzCore/Module/Module.h>
#include <Components/ExampleFilteredEntityComponent.h>
#include <Components/PerfTest/NetworkPrefabSpawnerComponent.h>
#include <Components/PerfTest/NetworkRandomImpulseComponent.h>
#include <Components/PerfTest/NetworkTestSpawnerComponent.h>
#include <Components/UI/HUDComponent.h>
#include <Components/UI/MatchOverComponent.h>
#include <Components/UI/UiCoinCountComponent.h>
#include <Components/UI/UiMatchPlayerCoinCountsComponent.h>
#include <Source/AutoGen/AutoComponentTypes.h>

#include "MultiplayerSampleSystemComponent.h"

namespace MultiplayerSample
{
    class MultiplayerSampleModule
        : public AZ::Module
    {
    public:
        AZ_RTTI(MultiplayerSampleModule, "{9323FFB1-54AB-4665-889B-166CA2418C7C}", AZ::Module);
        AZ_CLASS_ALLOCATOR(MultiplayerSampleModule, AZ::SystemAllocator, 0);

        MultiplayerSampleModule()
            : AZ::Module()
        {
            // Push results of [MyComponent]::CreateDescriptor() into m_descriptors here.
            m_descriptors.insert(m_descriptors.end(), {
                MultiplayerSampleSystemComponent::CreateDescriptor(),
                ExampleFilteredEntityComponent::CreateDescriptor(),
                HUDComponent::CreateDescriptor(),
                MatchOverComponent::CreateDescriptor(),
                NetworkPrefabSpawnerComponent::CreateDescriptor(),
                UiCoinCountComponent::CreateDescriptor(),
                UiMatchPlayerCoinCountsComponent::CreateDescriptor(),
            });

            CreateComponentDescriptors(m_descriptors);
        }

        /**
         * Add required SystemComponents to the SystemEntity.
         */
        AZ::ComponentTypeList GetRequiredSystemComponents() const override
        {
            return AZ::ComponentTypeList{
                azrtti_typeid<MultiplayerSampleSystemComponent>(),
            };
        }
    };
}

// DO NOT MODIFY THIS LINE UNLESS YOU RENAME THE GEM
// The first parameter should be GemName_GemIdLower
// The second should be the fully qualified name of the class above
AZ_DECLARE_MODULE_CLASS(Gem_MultiplayerSample, MultiplayerSample::MultiplayerSampleModule)
