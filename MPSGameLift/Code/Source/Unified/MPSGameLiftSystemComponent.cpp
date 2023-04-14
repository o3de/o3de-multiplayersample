/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include "MPSGameLiftSystemComponent.h"

#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/EditContextConstants.inl>

namespace MPSGameLift
{
    void MPSGameLiftSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serialize = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serialize->Class<MPSGameLiftSystemComponent, AZ::Component>()
                ->Version(0)
                ;

            if (AZ::EditContext* ec = serialize->GetEditContext())
            {
                ec->Class<MPSGameLiftSystemComponent>("MPSGameLift", "[Description of functionality provided by this System Component]")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ;
            }
        }
    }

    void MPSGameLiftSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("MPSGameLiftService"));
    }

    void MPSGameLiftSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("MPSGameLiftService"));
    }

    void MPSGameLiftSystemComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
    }

    void MPSGameLiftSystemComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
    }

    MPSGameLiftSystemComponent::MPSGameLiftSystemComponent()
    {
        if (MPSGameLiftInterface::Get() == nullptr)
        {
            MPSGameLiftInterface::Register(this);
        }
    }

    MPSGameLiftSystemComponent::~MPSGameLiftSystemComponent()
    {
        if (MPSGameLiftInterface::Get() == this)
        {
            MPSGameLiftInterface::Unregister(this);
        }
    }

    void MPSGameLiftSystemComponent::Init()
    {
    }

    void MPSGameLiftSystemComponent::Activate()
    {
        MPSGameLiftRequestBus::Handler::BusConnect();
    }

    void MPSGameLiftSystemComponent::Deactivate()
    {
        MPSGameLiftRequestBus::Handler::BusDisconnect();
    }

} // namespace MPSGameLift
