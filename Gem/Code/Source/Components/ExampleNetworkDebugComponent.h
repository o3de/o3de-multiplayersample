/*
 * Copyright (c) Contributors to the Open 3D Engine Project
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/Component/Component.h>
#include <AzCore/Component/TransformBus.h>

namespace MultiplayerSample
{
    class ExampleNetworkDebugComponent final
        : public AZ::Component
        , public AZ::TransformNotificationBus::Handler
    {
    public:
        AZ_COMPONENT(MultiplayerSample::ExampleNetworkDebugComponent, "{A7A086A3-1F9A-44EC-8FE4-3792D52DC889}");

        static void Reflect(AZ::ReflectContext* context);

        //! AZ::Component overrides.
        //! @{
        void Activate() override;
        void Deactivate() override;
        //! }@

        //! AZ::TransformNotificationBus::Handler overrides.
        //! @{
        void OnTransformChanged(const AZ::Transform& local, const AZ::Transform& world) override;
        //! }@

        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
        {
            required.push_back(AZ_CRC_CE("TransformService"));
        }
    };
}
