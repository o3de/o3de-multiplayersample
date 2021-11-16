/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AzCore/Math/Uuid.h>
#include <AzCore/Interface/Interface.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/EditContextConstants.inl>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzFramework/Session/SessionConfig.h>

#include "MPSGameLiftClientComponent.h"
#include <AWSGameLiftPlayer.h>
#include <Request/AWSGameLiftAcceptMatchRequest.h>
#include <Request/AWSGameLiftCreateSessionOnQueueRequest.h>
#include <Request/AWSGameLiftCreateSessionRequest.h>
#include <Request/AWSGameLiftJoinSessionRequest.h>
#include <Request/AWSGameLiftSearchSessionsRequest.h>
#include <Request/AWSGameLiftStartMatchmakingRequest.h>
#include <Request/AWSGameLiftStopMatchmakingRequest.h>
#include <ResourceMapping/AWSResourceMappingBus.h>

namespace MultiplayerSample
{
    AZ_CVAR(bool, cl_joinaftersearch, false, nullptr, AZ::ConsoleFunctorFlags::DontReplicate,
        "Whether to join the first available session after search automatically");
    AZ_CVAR(bool, cl_acceptmatch, false, nullptr, AZ::ConsoleFunctorFlags::DontReplicate,
        "Whether to accept the match automatically");

    void MPSGameLiftClientSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serialize = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serialize->Class<MPSGameLiftClientSystemComponent, AZ::Component>()->Version(0);

            if (AZ::EditContext* ec = serialize->GetEditContext())
            {
                ec->Class<MPSGameLiftClientSystemComponent>(
                      "MPSGameLiftClientSystemComponent", "System Component for the GameLift client")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("System"))
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true);
            }
        }
    }

    void MPSGameLiftClientSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("MultiplayerSampleGameLiftService"));
    }

    void MPSGameLiftClientSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("MultiplayerSampleGameLiftService"));
    }

    void MPSGameLiftClientSystemComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        required.push_back(AZ_CRC_CE("AWSGameLiftClientService"));
    }

    void MPSGameLiftClientSystemComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        AZ_UNUSED(dependent);
    }

    void MPSGameLiftClientSystemComponent::Init()
    {
        m_playerId = AZ::Uuid().Create().ToString<AZStd::string>();
    }

    void MPSGameLiftClientSystemComponent::Activate()
    {
        AzFramework::SessionAsyncRequestNotificationBus::Handler::BusConnect();
        AzFramework::MatchmakingAsyncRequestNotificationBus::Handler::BusConnect();

        AWSGameLift::AWSGameLiftRequestBus::Broadcast(&AWSGameLift::AWSGameLiftRequestBus::Events::ConfigureGameLiftClient, "");
    }

    void MPSGameLiftClientSystemComponent::Deactivate()
    {
        m_ticketId = "";

        AzFramework::MatchmakingAsyncRequestNotificationBus::Handler::BusDisconnect();
        AzFramework::SessionAsyncRequestNotificationBus::Handler::BusDisconnect();
    }

    void MPSGameLiftClientSystemComponent::HostSession(const AZ::ConsoleCommandContainer& consoleFunctionParameters)
    {
        if (consoleFunctionParameters.size() != 2)
        {
            AZ_Error("MPSGameLiftClientSystemComponent", false, "Invalid console command. Use HostSession <session-id> <max-player>");
            return;
        }

        AWSGameLift::AWSGameLiftCreateSessionRequest request;
        request.m_idempotencyToken = consoleFunctionParameters[0];
        request.m_maxPlayer = AZStd::stoi(AZStd::string(consoleFunctionParameters[1]));

        AWSCore::AWSResourceMappingRequestBus::BroadcastResult(request.m_fleetId,
            &AWSCore::AWSResourceMappingRequestBus::Events::GetResourceNameId, "MultiplayerSampleFleetId");
        if (request.m_fleetId.empty())
        {
            AZ_Error("MPSGameLiftClientSystemComponent", false, "Failed to get GameLift fleet Id from resource mapping file");
            return;
        }

        AWSGameLift::AWSGameLiftSessionAsyncRequestBus::Broadcast(
            &AWSGameLift::AWSGameLiftSessionAsyncRequestBus::Events::CreateSessionAsync, request);
    }

    void MPSGameLiftClientSystemComponent::HostSessionOnQueue(const AZ::ConsoleCommandContainer& consoleFunctionParameters)
    {
        if (consoleFunctionParameters.size() != 2)
        {
            AZ_Error("MPSGameLiftClientSystemComponent", false, "Invalid console command. Use HostSessionOnQueue <placement-id> <max-player>");
            return;
        }

        AWSGameLift::AWSGameLiftCreateSessionOnQueueRequest request;
        request.m_placementId = consoleFunctionParameters[0];
        request.m_maxPlayer = AZStd::stoi(AZStd::string(consoleFunctionParameters[1]));

        AWSCore::AWSResourceMappingRequestBus::BroadcastResult(request.m_queueName,
            &AWSCore::AWSResourceMappingRequestBus::Events::GetResourceNameId, "MultiplayerSampleQueueName");
        if (request.m_queueName.empty())
        {
            AZ_Error("MPSGameLiftClientSystemComponent", false, "Failed to get GameLift queue name from resource mapping file");
            return;
        }

        AWSGameLift::AWSGameLiftSessionAsyncRequestBus::Broadcast(
            &AWSGameLift::AWSGameLiftSessionAsyncRequestBus::Events::CreateSessionAsync, request);
    }

    void MPSGameLiftClientSystemComponent::JoinSession(const AZ::ConsoleCommandContainer& consoleFunctionParameters)
    {
        if (consoleFunctionParameters.size() != 1)
        {
            AZ_Error("MPSGameLiftClientSystemComponent", false, "Invalid console command. Use JoinSession <session-id>");
            return;
        }

        JoinSessionInternal(consoleFunctionParameters[0], m_playerId);
    }

    void MPSGameLiftClientSystemComponent::SearchSessions(const AZ::ConsoleCommandContainer& consoleFunctionParameters)
    {
        if (consoleFunctionParameters.size() > 0)
        {
            AZ_Error("MPSGameLiftClientSystemComponent", false, "Invalid console command. Use SearchSessions");
            return;
        }

        AWSGameLift::AWSGameLiftSearchSessionsRequest request;
        request.m_maxResult = 0;     

        AWSCore::AWSResourceMappingRequestBus::BroadcastResult(request.m_fleetId,
            &AWSCore::AWSResourceMappingRequestBus::Events::GetResourceNameId, "MultiplayerSampleFleetId");
        if (request.m_fleetId.empty())
        {
            AZ_Error("MPSGameLiftClientSystemComponent", false, "Failed to get GameLift fleet Id from resource mapping file");
            return;
        }
        
        AWSGameLift::AWSGameLiftSessionAsyncRequestBus::Broadcast(
            &AWSGameLift::AWSGameLiftSessionAsyncRequestBus::Events::SearchSessionsAsync, request);
    }

    void MPSGameLiftClientSystemComponent::LeaveSession(const AZ::ConsoleCommandContainer& consoleFunctionParameters)
    {
        if (consoleFunctionParameters.size() > 1)
        {
            AZ_Error("MPSGameLiftClientSystemComponent", false, "Invalid console command. Use LeaveSession");
            return;
        }

        AWSGameLift::AWSGameLiftSessionAsyncRequestBus::Broadcast(
            &AWSGameLift::AWSGameLiftSessionAsyncRequestBus::Events::LeaveSessionAsync);
    }

    void MPSGameLiftClientSystemComponent::StartMatchmaking(const AZ::ConsoleCommandContainer& consoleFunctionParameters)
    {
        if (consoleFunctionParameters.size() > 1)
        {
            AZ_Error("MPSGameLiftClientSystemComponent", false, "Invalid console command. Use StartMatchmaking <ticket-id> or StartMatchmaking");
            return;
        }

        AWSGameLift::AWSGameLiftStartMatchmakingRequest request;
        if (consoleFunctionParameters.size() == 1)
        {
            request.m_ticketId = consoleFunctionParameters[0];
        }

        AWSGameLift::AWSGameLiftPlayer player;
        player.m_playerId = m_playerId;
        request.m_players = { player };

        AWSCore::AWSResourceMappingRequestBus::BroadcastResult(request.m_configurationName,
            &AWSCore::AWSResourceMappingRequestBus::Events::GetResourceNameId, "MultiplayerSampleConfigurationName");
        if (request.m_configurationName.empty())
        {
            AZ_Error("MPSGameLiftClientSystemComponent", false, "Failed to get GameLift matchmaking configuration name from resource mapping file");
            return;
        }

        AWSGameLift::AWSGameLiftMatchmakingAsyncRequestBus::Broadcast(
            &AWSGameLift::AWSGameLiftMatchmakingAsyncRequestBus::Events::StartMatchmakingAsync, request);
    }

    void MPSGameLiftClientSystemComponent::StopMatchmaking(const AZ::ConsoleCommandContainer& consoleFunctionParameters)
    {
        if (consoleFunctionParameters.size() > 0)
        {
            AZ_Error("MPSGameLiftClientSystemComponent", false, "Invalid console command. Use StopMatchmaking");
            return;
        }

        if (m_ticketId.empty())
        {
            AZ_Error("MPSGameLiftClientSystemComponent", false, "No active matchmaking ticket to stop");
            return;
        }

        AWSGameLift::AWSGameLiftStopMatchmakingRequest request;
        request.m_ticketId = m_ticketId;

        AWSGameLift::AWSGameLiftMatchmakingAsyncRequestBus::Broadcast(
            &AWSGameLift::AWSGameLiftMatchmakingAsyncRequestBus::Events::StopMatchmakingAsync, request);
    }

    void MPSGameLiftClientSystemComponent::OnCreateSessionAsyncComplete(const AZStd::string& createSessionReponse)
    {
        AZ_TracePrintf("MPSGameLiftClientSystemComponent", "CreateSessionAsync complete with result: %s", createSessionReponse.c_str());
    }

    void MPSGameLiftClientSystemComponent::JoinSessionInternal(const AZStd::string& sessionId, const AZStd::string& playerId)
    {
        AWSGameLift::AWSGameLiftJoinSessionRequest request;
        request.m_sessionId = sessionId;
        request.m_playerId = playerId;

        AWSGameLift::AWSGameLiftSessionAsyncRequestBus::Broadcast(
            &AWSGameLift::AWSGameLiftSessionAsyncRequestBus::Events::JoinSessionAsync, request);
    }

    void MPSGameLiftClientSystemComponent::OnSearchSessionsAsyncComplete(const AzFramework::SearchSessionsResponse& searchSessionsResponse)
    {
        AZStd::vector<AZStd::string> searchResults;
        AZStd::string sessionIds = "";
        for (auto& sessionConfig : searchSessionsResponse.m_sessionConfigs)
        {
            sessionIds += AZStd::string::format("SessionId=%s, ", sessionConfig.m_sessionId.c_str());
            searchResults.push_back(sessionConfig.m_sessionId);
        }
        if (!sessionIds.empty())
        {
            sessionIds = sessionIds.substr(0, sessionIds.size() - 2);
        }

        AZ_TracePrintf("MPSGameLiftClientSystemComponent", "SearchSessionsAsync complete with result: %s",
            AZStd::string::format("[%s]", sessionIds.c_str()).c_str());

        if (cl_joinaftersearch && !sessionIds.empty())
        {
            AZ_TracePrintf("MPSGameLiftClientSystemComponent", "Start joining session: %s", searchResults[0].c_str());

            JoinSessionInternal(searchResults[0], m_playerId);
        }
        else
        {
            AZ_Error("MPSGameLiftClientSystemComponent", false, "No existing session to join");
        }
    }

    void MPSGameLiftClientSystemComponent::OnJoinSessionAsyncComplete(bool joinSessionsResponse)
    {
        if (joinSessionsResponse)
        {
            AZ_TracePrintf("MPSGameLiftClientSystemComponent", "JoinSessionAsync complete with result: success");
        }
        else
        {
            AZ_TracePrintf("MPSGameLiftClientSystemComponent", "JoinSessionAsync complete with result: fail");
        }
    }

    void MPSGameLiftClientSystemComponent::OnLeaveSessionAsyncComplete()
    {
        AZ_TracePrintf("MPSGameLiftClientSystemComponent", "LeaveSessionAsync complete");

        m_ticketId = "";
    }

    void MPSGameLiftClientSystemComponent::OnAcceptMatchAsyncComplete()
    {
        AZ_TracePrintf("MPSGameLiftClientSystemComponent", "AcceptMatchAsync complete");
    }

    void MPSGameLiftClientSystemComponent::OnStartMatchmakingAsyncComplete(const AZStd::string& matchmakingTicketId)
    {
        AZ_TracePrintf("MPSGameLiftClientSystemComponent", "StartMatchmakingAsync complete with ticket ID: %s", matchmakingTicketId.c_str());

        m_ticketId = matchmakingTicketId;
        AWSGameLift::AWSGameLiftMatchmakingEventRequestBus::Broadcast(
            &AWSGameLift::AWSGameLiftMatchmakingEventRequestBus::Events::StartPolling, m_ticketId, m_playerId);
    }

    void MPSGameLiftClientSystemComponent::OnStopMatchmakingAsyncComplete()
    {
        AZ_TracePrintf("MultiplayerSample", "StopMatchmakingAsync complete");

        m_ticketId = "";
        AWSGameLift::AWSGameLiftMatchmakingEventRequestBus::Broadcast(
            &AWSGameLift::AWSGameLiftMatchmakingEventRequestBus::Events::StopPolling);
    }

    void MPSGameLiftClientSystemComponent::OnMatchAcceptance()
    {
        if (cl_acceptmatch)
        {
            AcceptMatch();
        }
        else
        {
            AZ_TracePrintf("MPSGameLiftClientSystemComponent", "A match is found. Please accept or reject it");
        }       
    }

    void MPSGameLiftClientSystemComponent::AcceptMatch(bool accept)
    {
        if (m_ticketId.empty())
        {
            AZ_Error("MPSGameLiftClientSystemComponent", false, "No active matchmaking ticket to accept");
            return;
        }

        AWSGameLift::AWSGameLiftAcceptMatchRequest request;
        request.m_acceptMatch = accept;
        request.m_ticketId = m_ticketId;
        request.m_playerIds = { m_playerId };

        AWSGameLift::AWSGameLiftMatchmakingAsyncRequestBus::Broadcast(
            &AWSGameLift::AWSGameLiftMatchmakingAsyncRequestBus::Events::AcceptMatchAsync, request);
    }

    void MPSGameLiftClientSystemComponent::OnMatchComplete()
    {
    }

    void MPSGameLiftClientSystemComponent::OnMatchError()
    {
    }

    void MPSGameLiftClientSystemComponent::OnMatchFailure()
    {
    }
} // namespace MultiplayerSample
