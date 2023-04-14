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
#include <MPSGameLift/MPSGameLiftBus.h>

namespace MPSGameLift
{
    class MPSGameLiftSystemComponent
        : public AZ::Component
        , protected MPSGameLiftRequestBus::Handler
    {
    public:
        AZ_COMPONENT(MPSGameLiftSystemComponent, "{FEDB720B-2B15-423D-8596-41E9E3FBBC3B}");

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        MPSGameLiftSystemComponent();
        ~MPSGameLiftSystemComponent();

    protected:
        ////////////////////////////////////////////////////////////////////////
        // MPSGameLiftRequestBus interface implementation

        ////////////////////////////////////////////////////////////////////////

        ////////////////////////////////////////////////////////////////////////
        // AZ::Component interface implementation
        void Init() override;
        void Activate() override;
        void Deactivate() override;
        ////////////////////////////////////////////////////////////////////////
    };

} // namespace MPSGameLift
