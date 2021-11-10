/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */
#pragma once

#include <AzCore/Component/Component.h>
#include <AzCore/Console/IConsole.h>
#include <AzFramework/Matchmaking/MatchmakingNotifications.h>

#include <Request/AWSGameLiftRequestBus.h>
#include <Request/AWSGameLiftSessionRequestBus.h>
#include <Request/AWSGameLiftMatchmakingRequestBus.h>


namespace MultiplayerSample
{
    class GameLiftClientSystemComponent
        : public AZ::Component
        , public AzFramework::SessionAsyncRequestNotificationBus::Handler
        , public AzFramework::MatchmakingNotificationBus::Handler
        , public AzFramework::MatchmakingAsyncRequestNotificationBus::Handler
    {
    public:
        AZ_COMPONENT(GameLiftClientSystemComponent, "{F83C5B49-3E33-4BB9-B536-0232D310C298}");

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

    protected:
        ////////////////////////////////////////////////////////////////////////
        // AZ::Component interface implementation
        void Init() override;
        void Activate() override;
        void Deactivate() override;

        void HostSession(const AZ::ConsoleCommandContainer& consoleFunctionParameters);
        AZ_CONSOLEFUNC(GameLiftClientSystemComponent, HostSession, AZ::ConsoleFunctorFlags::DontReplicate, "Host and join a new session");

        void HostSessionOnQueue(const AZ::ConsoleCommandContainer& consoleFunctionParameters);
        AZ_CONSOLEFUNC(GameLiftClientSystemComponent, HostSessionOnQueue, AZ::ConsoleFunctorFlags::DontReplicate, "Host and join a new session on queue");

        void JoinSession(const AZ::ConsoleCommandContainer& consoleFunctionParameters);
        AZ_CONSOLEFUNC(GameLiftClientSystemComponent, JoinSession, AZ::ConsoleFunctorFlags::DontReplicate, "Join an existing session");

        void SearchSessions(const AZ::ConsoleCommandContainer& consoleFunctionParameters);
        AZ_CONSOLEFUNC(GameLiftClientSystemComponent, SearchSessions, AZ::ConsoleFunctorFlags::DontReplicate, "Search for existing sessions");

        void LeaveSession(const AZ::ConsoleCommandContainer& consoleFunctionParameters);
        AZ_CONSOLEFUNC(GameLiftClientSystemComponent, LeaveSession, AZ::ConsoleFunctorFlags::DontReplicate, "Leave the current session");

        void StartMatchmaking(const AZ::ConsoleCommandContainer& consoleFunctionParameters);
        AZ_CONSOLEFUNC(GameLiftClientSystemComponent, StartMatchmaking, AZ::ConsoleFunctorFlags::DontReplicate, "Start a matchmaking request");

        void StopMatchmaking(const AZ::ConsoleCommandContainer& consoleFunctionParameters);
        AZ_CONSOLEFUNC(GameLiftClientSystemComponent, StopMatchmaking, AZ::ConsoleFunctorFlags::DontReplicate, "Stop a matchmaking request");

        ////////////////////////////////////////////////////////////////////////
        // SessionAsyncRequestNotificationBus handler implementation
        void OnCreateSessionAsyncComplete(const AZStd::string& createSessionReponse) override;

        void OnSearchSessionsAsyncComplete(const AzFramework::SearchSessionsResponse& searchSessionsResponse) override;

        void OnJoinSessionAsyncComplete(bool joinSessionsResponse) override;

        void OnLeaveSessionAsyncComplete() override;

        ////////////////////////////////////////////////////////////////////////
        //MatchmakingAsyncRequestNotificationBus handler implementation
        void OnAcceptMatchAsyncComplete() override;

        void OnStartMatchmakingAsyncComplete(const AZStd::string& matchmakingTicketId) override;

        void OnStopMatchmakingAsyncComplete() override;

        ////////////////////////////////////////////////////////////////////////
        //MatchmakingNotificationBus handler implementation
        void OnMatchAcceptance() override;
        void OnMatchComplete() override;
        void OnMatchError() override;
        void OnMatchFailure() override;

         ////////////////////////////////////////////////////////////////////////

    private:
        void AcceptMatch(bool accept = true);
        void JoinSessionInternal(const AZStd::string& sessionId, const AZStd::string& playerId);

        AZStd::string m_ticketId;
        AZStd::string m_playerId;
    };
} // namespace MultiplayerSample#pragma once
