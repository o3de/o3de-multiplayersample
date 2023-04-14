/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 * 
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/Component/Component.h>
#include <AzFramework/API/ApplicationAPI.h>
#include <Multiplayer/Session/SessionNotifications.h>

namespace MPSGameLift
{
    class MPSGameLiftServerSystemComponent
        : public AZ::Component
        , public Multiplayer::SessionNotificationBus::Handler
        , public AzFramework::LevelLoadBlockerBus::Handler
    {
    public: 
        AZ_COMPONENT(MPSGameLiftServerSystemComponent, "{2D3C2443-1F3E-477E-8C26-506E62972E67}");

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);

    protected:
        // AZ::Component interface implementation
        void Init() override;
        void Activate() override;
        void Deactivate() override;

        // Multiplayer::SessionNotificationBus::Handler overrides
        bool OnSessionHealthCheck() override;
        bool OnCreateSessionBegin(const Multiplayer::SessionConfig&) override { return true; }
        void OnCreateSessionEnd() override;
        bool OnDestroySessionBegin() override { return true; }
        void OnDestroySessionEnd() override {}
        void OnUpdateSessionBegin(const Multiplayer::SessionConfig&, [[maybe_unused]] const AZStd::string& updateReason) override {}
        void OnUpdateSessionEnd() override {}

        // AzFramework::LevelLoadBlockerBus::Handler overrides
        bool ShouldBlockLevelLoading(const char* levelName) override;

        AZStd::string m_loadedLevelName = "";
    };
}
