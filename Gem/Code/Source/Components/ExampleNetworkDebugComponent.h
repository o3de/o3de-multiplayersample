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
