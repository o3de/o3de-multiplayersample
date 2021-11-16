/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 * 
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AzCore/Memory/SystemAllocator.h>
#include <AzCore/Module/Module.h>
#include <Source/AutoGen/AutoComponentTypes.h>

#include "MultiplayerSampleSystemComponent.h"
#include "GameLift/MPSGameLiftServerComponent.h"

namespace MultiplayerSample
{
    class MPSGameLiftServerModule
        : public AZ::Module
    {
    public:
        AZ_RTTI(MPSGameLiftServerModule, "{C70DF768-3BB1-46BC-9F96-DE86B53610D2}", AZ::Module);
        AZ_CLASS_ALLOCATOR(MPSGameLiftServerModule, AZ::SystemAllocator, 0);

        MPSGameLiftServerModule()
            : AZ::Module()
        {
            // Push results of [MyComponent]::CreateDescriptor() into m_descriptors here.
            m_descriptors.insert(m_descriptors.end(), {
                MultiplayerSampleSystemComponent::CreateDescriptor(),
                    MPSGameLiftServerSystemComponent::CreateDescriptor(),
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
                azrtti_typeid<MPSGameLiftServerSystemComponent>(),
            };
        }
    };
}

// DO NOT MODIFY THIS LINE UNLESS YOU RENAME THE GEM
// The first parameter should be GemName_GemIdLower
// The second should be the fully qualified name of the class above
AZ_DECLARE_MODULE_CLASS(Gem_MPSMultiplayerSample_GameLiftServer, MultiplayerSample::MPSGameLiftServerModule)
