/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AzCore/Serialization/SerializeContext.h>
#include "MPSGameLiftEditorSystemComponent.h"

namespace MPSGameLift
{
    void MPSGameLiftEditorSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<MPSGameLiftEditorSystemComponent, MPSGameLiftSystemComponent>()
                ->Version(0);
        }
    }

    MPSGameLiftEditorSystemComponent::MPSGameLiftEditorSystemComponent() = default;

    MPSGameLiftEditorSystemComponent::~MPSGameLiftEditorSystemComponent() = default;

    void MPSGameLiftEditorSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        BaseSystemComponent::GetProvidedServices(provided);
        provided.push_back(AZ_CRC_CE("MPSGameLiftEditorService"));
    }

    void MPSGameLiftEditorSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        BaseSystemComponent::GetIncompatibleServices(incompatible);
        incompatible.push_back(AZ_CRC_CE("MPSGameLiftEditorService"));
    }

    void MPSGameLiftEditorSystemComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        BaseSystemComponent::GetRequiredServices(required);
    }

    void MPSGameLiftEditorSystemComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        BaseSystemComponent::GetDependentServices(dependent);
    }

    void MPSGameLiftEditorSystemComponent::Activate()
    {
        MPSGameLiftSystemComponent::Activate();
        AzToolsFramework::EditorEvents::Bus::Handler::BusConnect();
    }

    void MPSGameLiftEditorSystemComponent::Deactivate()
    {
        AzToolsFramework::EditorEvents::Bus::Handler::BusDisconnect();
        MPSGameLiftSystemComponent::Deactivate();
    }

} // namespace MPSGameLift
