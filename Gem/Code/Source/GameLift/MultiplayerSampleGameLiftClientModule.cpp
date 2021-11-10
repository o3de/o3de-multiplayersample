/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 * 
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AzCore/Memory/SystemAllocator.h>
#include <AzCore/Module/Module.h>
#include <Components/ExampleFilteredEntityComponent.h>
#include <Source/AutoGen/AutoComponentTypes.h>

#include "MultiplayerSampleSystemComponent.h"
#include "GameLift/GameLiftClientComponent.h"

namespace MultiplayerSample
{
    class GameLiftClientModule
        : public AZ::Module
    {
    public:
        AZ_RTTI(GameLiftClientModule, "{7D696A0B-8405-4D60-AC33-6AE24BD00F38}", AZ::Module);
        AZ_CLASS_ALLOCATOR(GameLiftClientModule, AZ::SystemAllocator, 0);

        GameLiftClientModule()
            : AZ::Module()
        {
            // Push results of [MyComponent]::CreateDescriptor() into m_descriptors here.
            m_descriptors.insert(m_descriptors.end(), {
                MultiplayerSampleSystemComponent::CreateDescriptor(),
                ExampleFilteredEntityComponent::CreateDescriptor(),
                GameLiftClientSystemComponent::CreateDescriptor(),
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
                azrtti_typeid<GameLiftClientSystemComponent>(),
            };
        }
    };
}

// DO NOT MODIFY THIS LINE UNLESS YOU RENAME THE GEM
// The first parameter should be GemName_GemIdLower
// The second should be the fully qualified name of the class above
AZ_DECLARE_MODULE_CLASS(Gem_MultiplayerSample_GameLiftClient, MultiplayerSample::GameLiftClientModule)
