/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AzCore/Memory/SystemAllocator.h>
#include <AzCore/Module/Module.h>
#include <Components/AttachPlayerWeaponComponent.h>
#include <Components/ExampleFilteredEntityComponent.h>
#include <Components/PerfTest/NetworkPrefabSpawnerComponent.h>
#include <Components/UI/UiCoinCountComponent.h>
#include <Components/UI/UiGameOverComponent.h>
#include <Components/UI/UiPlayerArmorComponent.h>
#include <Components/BackgroundMusicComponent.h>
#include <Components/ScriptableDecalComponent.h>
#include <Source/AutoGen/AutoComponentTypes.h>
#include <MultiplayerSampleSystemComponent.h>

#if AZ_TRAIT_CLIENT
#   include <Components/UI/HUDComponent.h>
#   include <Components/UI/UiMatchPlayerCoinCountsComponent.h>
#   include <Components/UI/UiRestBetweenRoundsComponent.h>
#   include <Components/UI/UiSettingsComponent.h>
#   include <Components/UI/UiStartMenuComponent.h>
    #include <UserSettings/MultiplayerSampleUserSettings.h>
#endif

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
                AttachPlayerWeaponComponent::CreateDescriptor(),
                ExampleFilteredEntityComponent::CreateDescriptor(),
                NetworkPrefabSpawnerComponent::CreateDescriptor(),
                UiCoinCountComponent::CreateDescriptor(),
                BackgroundMusicComponent::CreateDescriptor(),
                ScriptableDecalComponent::CreateDescriptor(),
                #if AZ_TRAIT_CLIENT
                    HUDComponent::CreateDescriptor(),
                    UiGameOverComponent::CreateDescriptor(),
                    UiPlayerArmorComponent::CreateDescriptor(),
                    UiMatchPlayerCoinCountsComponent::CreateDescriptor(),
                    UiRestBetweenRoundsComponent::CreateDescriptor(),
                    UiSettingsComponent::CreateDescriptor(),
                    UiStartMenuComponent::CreateDescriptor(),
                #endif
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

#if AZ_TRAIT_CLIENT
        // This needs to be created as a part of the MultiplayerSampleModule, not during any sort of System Component activation.
        // It will affect registry keys that get read by System Components as a part of their activation and we can't guarantee
        // that those other core System Components will get started after our game-specific one.
        MultiplayerSampleUserSettings m_userSettings;
#endif
    };
}

// DO NOT MODIFY THIS LINE UNLESS YOU RENAME THE GEM
// The first parameter should be GemName_GemIdLower
// The second should be the fully qualified name of the class above
#if defined(AZ_MONOLITHIC_BUILD)
AZ_DECLARE_MODULE_CLASS(Gem_MultiplayerSample_Client, MultiplayerSample::MultiplayerSampleModule);
AZ_DECLARE_MODULE_CLASS(Gem_MultiplayerSample_Server, MultiplayerSample::MultiplayerSampleModule);
#endif
AZ_DECLARE_MODULE_CLASS(Gem_MultiplayerSample, MultiplayerSample::MultiplayerSampleModule)
