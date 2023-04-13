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
#include <Components/ScriptableDecalComponent.h>
#if AZ_TRAIT_CLIENT
    #include <Components/UI/HUDComponent.h>
    #include <Components/UI/UiMatchPlayerCoinCountsComponent.h>
    #include <Components/UI/UiRestBetweenRoundsComponent.h>
    #include <Components/UI/UiStartMenuComponent.h>
    #include <Source/MultiplayerSampleAWSGameLiftClientSystemComponent.h>
#endif
#if AZ_TRAIT_SERVER  && !AZ_TRAIT_CLIENT
    #include <Source/MultiplayerSampleAWSGameLiftServerSystemComponent.h>
    #include <AzCore/Console/IConsole.h>
#endif

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
                AttachPlayerWeaponComponent::CreateDescriptor(),
                ExampleFilteredEntityComponent::CreateDescriptor(),
                NetworkPrefabSpawnerComponent::CreateDescriptor(),
                UiCoinCountComponent::CreateDescriptor(),
                ScriptableDecalComponent::CreateDescriptor(),
                #if AZ_TRAIT_CLIENT
                    HUDComponent::CreateDescriptor(),
                    UiGameOverComponent::CreateDescriptor(),
                    UiPlayerArmorComponent::CreateDescriptor(),
                    UiMatchPlayerCoinCountsComponent::CreateDescriptor(),
                    UiRestBetweenRoundsComponent::CreateDescriptor(),
                    UiStartMenuComponent::CreateDescriptor(),
                    MultiplayerSampleAWSGameLiftClientSystemComponent::CreateDescriptor(),
                #endif
                #if AZ_TRAIT_SERVER && !AZ_TRAIT_CLIENT
                    MultiplayerSampleAWSGameLiftServerSystemComponent::CreateDescriptor(),
                #endif
            });

            CreateComponentDescriptors(m_descriptors);
        }

        /**
         * Add required SystemComponents to the SystemEntity.
         */
        AZ::ComponentTypeList GetRequiredSystemComponents() const override
        {
            AZ::ComponentTypeList requiredSystemComponents{
                azrtti_typeid<MultiplayerSampleSystemComponent>(),
                #if AZ_TRAIT_CLIENT
                    azrtti_typeid<MultiplayerSampleAWSGameLiftClientSystemComponent>(),
                #endif
            };

            // Only activate the MultiplayerSample AWS GameLift server system component if this a dedicated server running on GameLift.
            #if AZ_TRAIT_SERVER && !AZ_TRAIT_CLIENT
                if (const auto console = AZ::Interface<AZ::IConsole>::Get())
                {
                    bool sv_gameLiftEnabled = false;
                    if (console->GetCvarValue("sv_gameLiftEnabled", sv_gameLiftEnabled) == AZ::GetValueResult::Success)
                    {
                        if (sv_gameLiftEnabled)
                        {
                            requiredSystemComponents.push_back(azrtti_typeid<MultiplayerSampleAWSGameLiftServerSystemComponent>());
                        }
                    }
                    else
                    {
                        AZ_Assert(false, "MultiplayerSample expecting to access an invalid sv_gameLiftEnabled. Please update code to properly check if GameLift is enabled in order to enable it's custom GameLift server system component.")
                    }
                }
                else
                {
                    AZ_Assert(false, "MultiplayerSample expecting to check AZ::Console, but it's not available. Please update code to properly check if this server is running on GameLift.")
                }
            #endif
            
            return requiredSystemComponents;
        }
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
