/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 * 
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/Component/Component.h>
#include <Multiplayer/Session/SessionNotifications.h>

namespace MultiplayerSample
{
    class MultiplayerSampleAWSGameLiftServerSystemComponent
        : public AZ::Component
        , public Multiplayer::SessionNotificationBus::Handler
    {
    public: 
        AZ_COMPONENT(MultiplayerSampleAWSGameLiftServerSystemComponent, "{0b0d2c48-058d-4207-b2c5-2778e50ec1c9}");

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

    protected:
        // AZ::Component interface implementation
        void Init() override;
        void Activate() override;
        void Deactivate() override;

        // Multiplayer::SessionNotificationBus::Handler overrides
        bool OnSessionHealthCheck() override;
        bool OnCreateSessionBegin(const Multiplayer::SessionConfig& sessionConfig) override;
        void OnCreateSessionEnd() override {};
        bool OnDestroySessionBegin() override { return true; };
        void OnDestroySessionEnd() override {};
        void OnUpdateSessionBegin(const Multiplayer::SessionConfig&, [[maybe_unused]] const AZStd::string& updateReason) override {};
        void OnUpdateSessionEnd() override {};
    };
}