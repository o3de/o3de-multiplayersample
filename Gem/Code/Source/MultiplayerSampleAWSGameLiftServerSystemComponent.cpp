/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 * 
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include "MultiplayerSampleAWSGameLiftServerSystemComponent.h"

#include <AzCore/Console/IConsole.h>
#include <AzCore/Interface/Interface.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/EditContextConstants.inl>

#include <Request/AWSGameLiftServerRequestBus.h>

namespace MultiplayerSample
{
    void MultiplayerSampleAWSGameLiftServerSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serialize = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serialize->Class<MultiplayerSampleAWSGameLiftServerSystemComponent, AZ::Component>()
                ->Version(0)
                ;
        }
    }

    void MultiplayerSampleAWSGameLiftServerSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("MultiplayerSampleAWSGameLiftServerService"));
    }

    void MultiplayerSampleAWSGameLiftServerSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("MultiplayerSampleAWSGameLiftServerService"));
    }

    void MultiplayerSampleAWSGameLiftServerSystemComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        required.push_back(AZ_CRC_CE("MultiplayerService"));
        required.push_back(AZ_CRC_CE("AWSGameLiftServerService"));
    }

    void MultiplayerSampleAWSGameLiftServerSystemComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
    }

    void MultiplayerSampleAWSGameLiftServerSystemComponent::Init()
    {
    }

    void MultiplayerSampleAWSGameLiftServerSystemComponent::Activate()
    {
        Multiplayer::SessionNotificationBus::Handler::BusConnect();
        AzFramework::LevelLoadBlockerBus::Handler::BusConnect();

        AWSGameLift::AWSGameLiftServerRequestBus::Broadcast(
            &AWSGameLift::AWSGameLiftServerRequestBus::Events::NotifyGameLiftProcessReady);
    }

    void MultiplayerSampleAWSGameLiftServerSystemComponent::Deactivate()
    {
        Multiplayer::SessionNotificationBus::Handler::BusDisconnect();
        AzFramework::LevelLoadBlockerBus::Handler::BusDisconnect();
    }

    bool MultiplayerSampleAWSGameLiftServerSystemComponent::OnSessionHealthCheck()
    {
        // Add here: additional checks against game stats or other conditions, if needed, to determine session health.
        // For now, sufficient to return true so Amazon GameLift knows server process is responsive.
        return true;
    }

    bool MultiplayerSampleAWSGameLiftServerSystemComponent::OnCreateSessionBegin([[maybe_unused]] const Multiplayer::SessionConfig& sessionConfig)
    {
        // Add here: additional logic, if needed, to ready server process for hosting a session.

        AzFramework::LevelLoadBlockerBus::Handler::BusDisconnect();

        if (m_loadedLevelName != "")
        {
            AZ_Info("MultiplayerSampleAWSGameLiftServerSystemComponent", "Session requested by Amazon GameLift. Attempting to load level: '%s'", m_loadedLevelName.c_str());

            auto loadLevelCommand = AZStd::string::format("LoadLevel %s", m_loadedLevelName.c_str());
            AZ::Interface<AZ::IConsole>::Get()->PerformCommand(loadLevelCommand.c_str());
        }
        else
        {
            AZ_Info(
                "MultiplayerSampleAWSGameLiftServerSystemComponent",
                "Session requested by Amazon GameLift. Make sure to load into a multiplayer level before players join.");
        }

        return true;
    }

    bool MultiplayerSampleAWSGameLiftServerSystemComponent::ShouldBlockLevelLoading(const char* levelName)
    {
        m_loadedLevelName = levelName;
        if (levelName)
        {
            AZ_Info("MultiplayerSampleAWSGameLiftServerSystemComponent", "Interrupted load of level: '%s'", levelName);
        }
        else
        {
            AZ_Info("MultiplayerSampleAWSGameLiftServerSystemComponent", "Interrupted level load, but no level provided!");
        }
        
        return true;
    }
}
