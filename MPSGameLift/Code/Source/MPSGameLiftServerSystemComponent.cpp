/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 * 
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include "MPSGameLiftServerSystemComponent.h"

#include <AzCore/Console/IConsole.h>
#include <AzCore/Interface/Interface.h>
#include <AzCore/Serialization/SerializeContext.h>

#include <Request/AWSGameLiftServerRequestBus.h>

AZ_CVAR(
    float,
    sv_gameSessionNoPlayerShutdownTimeoutSeconds,
    3600.0f,
    nullptr,
    AZ::ConsoleFunctorFlags::DontReplicate,
    "The amount of seconds to wait before shutting down a game session if no players join."
);

namespace MPSGameLift
{
    void MPSGameLiftServerSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serialize = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serialize->Class<MPSGameLiftServerSystemComponent, AZ::Component>()
                ->Version(0)
                ;
        }
    }

    void MPSGameLiftServerSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("MPSGameLiftServerService"));
    }

    void MPSGameLiftServerSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("MPSGameLiftServerService"));
    }

    void MPSGameLiftServerSystemComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        required.push_back(AZ_CRC_CE("MultiplayerService"));
        required.push_back(AZ_CRC_CE("AWSGameLiftServerService"));
    }

    void MPSGameLiftServerSystemComponent::Init()
    {
    }

    void MPSGameLiftServerSystemComponent::Activate()
    {
        Multiplayer::SessionNotificationBus::Handler::BusConnect();
        AzFramework::LevelLoadBlockerBus::Handler::BusConnect();
        Multiplayer::GetMultiplayer()->AddConnectionAcquiredHandler(m_connectionAquiredEventHandler);

        AWSGameLift::AWSGameLiftServerRequestBus::Broadcast(
            &AWSGameLift::AWSGameLiftServerRequestBus::Events::NotifyGameLiftProcessReady);
    }

    void MPSGameLiftServerSystemComponent::Deactivate()
    {
        m_connectionAquiredEventHandler.Disconnect();
        Multiplayer::SessionNotificationBus::Handler::BusDisconnect();
        AzFramework::LevelLoadBlockerBus::Handler::BusDisconnect();
    }

    bool MPSGameLiftServerSystemComponent::OnSessionHealthCheck()
    {
        // Add here: additional checks against game stats or other conditions, if needed, to determine session health.
        // For now, sufficient to return true so Amazon GameLift knows server process is responsive.
        return true;
    }

    void MPSGameLiftServerSystemComponent::OnCreateSessionEnd()
    {
        AzFramework::LevelLoadBlockerBus::Handler::BusDisconnect();

        if (!m_loadedLevelName.empty())
        {
            AZ_Info("MPSGameLiftServerSystemComponent", "Session requested by Amazon GameLift. Attempting to load level: '%s'", m_loadedLevelName.c_str());

            auto loadLevelCommand = AZStd::string::format("LoadLevel %s", m_loadedLevelName.c_str());
            AZ::Interface<AZ::IConsole>::Get()->PerformCommand(loadLevelCommand.c_str());
        }
        else
        {
            AZ_Info(
                "MPSGameLiftServerSystemComponent",
                "Session requested by Amazon GameLift. Make sure to load into a multiplayer level before players join.");
        }

        // Start a timer to shutdown this server if no players join.
        // This scheduled event will be stopped if m_connectionAquiredEventHandler is triggered.
        m_gameSessionNoPlayerShutdown.Enqueue(AZ::SecondsToTimeMs(sv_gameSessionNoPlayerShutdownTimeoutSeconds));
    }

    bool MPSGameLiftServerSystemComponent::ShouldBlockLevelLoading(const char* levelName)
    {
        m_loadedLevelName = levelName;
        if (levelName)
        {
            AZ_Info("MPSGameLiftServerSystemComponent", "Interrupted load of level: '%s'", levelName);
        }
        else
        {
            AZ_Info("MPSGameLiftServerSystemComponent", "Interrupted level load, but no level provided!");
        }
        
        return true;
    }
}
