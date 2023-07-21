/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <MPSGameLiftModuleInterface.h>
#include "MPSGameLiftEditorSystemComponent.h"

namespace MPSGameLift
{
    class MPSGameLiftEditorModule
        : public MPSGameLiftModuleInterface
    {
    public:
        AZ_RTTI(MPSGameLiftEditorModule, "{83F11C38-6C62-49AC-B9AA-3AF337783A80}", MPSGameLiftModuleInterface);
        AZ_CLASS_ALLOCATOR(MPSGameLiftEditorModule, AZ::SystemAllocator, 0);

        MPSGameLiftEditorModule()
        {
            // Push results of [MyComponent]::CreateDescriptor() into m_descriptors here.
            // Add ALL components descriptors associated with this gem to m_descriptors.
            // This will associate the AzTypeInfo information for the components with the the SerializeContext, BehaviorContext and EditContext.
            // This happens through the [MyComponent]::Reflect() function.
            m_descriptors.insert(m_descriptors.end(), {
                MPSGameLiftEditorSystemComponent::CreateDescriptor(),
            });
        }

        /**
         * Add required SystemComponents to the SystemEntity.
         * Non-SystemComponents should not be added here
         */
        AZ::ComponentTypeList GetRequiredSystemComponents() const override
        {
            return AZ::ComponentTypeList {
                azrtti_typeid<MatchmakingSystemComponent>(),
                azrtti_typeid<MPSGameLiftEditorSystemComponent>(),
                azrtti_typeid<RegionalLatencySystemComponent>()
            };
        }
    };
}// namespace MPSGameLift

AZ_DECLARE_MODULE_CLASS(Gem_MPSGameLift, MPSGameLift::MPSGameLiftEditorModule)
