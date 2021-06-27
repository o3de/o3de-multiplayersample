/*
 * All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
 * its licensors.
 *
 * For complete copyright and license terms please see the LICENSE at the root of this
 * distribution (the "License"). All use of this software is governed by the License,
 * or, if provided, by the license below or the license accompanying this file. Do not
 * remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *
 */

#include <AzCore/Memory/SystemAllocator.h>
#include <AzCore/Module/Module.h>
#include <Components/ExampleFilteredEntityComponent.h>
#include <Components/ExampleNetworkDebugComponent.h>
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
                ExampleNetworkDebugComponent::CreateDescriptor(),
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
