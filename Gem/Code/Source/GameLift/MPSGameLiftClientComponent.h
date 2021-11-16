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
    //! System component for exposing GameLift client APIs as console functions and handling client notifications.
    class MPSGameLiftClientSystemComponent
        : public AZ::Component
        , public AzFramework::SessionAsyncRequestNotificationBus::Handler
        , public AzFramework::MatchmakingNotificationBus::Handler
        , public AzFramework::MatchmakingAsyncRequestNotificationBus::Handler
    {
    public:
        AZ_COMPONENT(MPSGameLiftClientSystemComponent, "{F83C5B49-3E33-4BB9-B536-0232D310C298}");

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

        //! Create a new game session on the GameLift fleet specified in the resource mapping configuration file directly
        //! Use console command MPSGameLiftClientSystemComponent.HostSession <session-id> <max-player>
        void HostSession(const AZ::ConsoleCommandContainer& consoleFunctionParameters);
        AZ_CONSOLEFUNC(MPSGameLiftClientSystemComponent, HostSession, AZ::ConsoleFunctorFlags::DontReplicate, "Host a new session");

        //! Create a new game session via the game session queue specified in the resource mapping configuration file
        //! Use console command MPSGameLiftClientSystemComponent.HostSessionOnQueue <placement-id> <max-player>
        void HostSessionOnQueue(const AZ::ConsoleCommandContainer& consoleFunctionParameters);
        AZ_CONSOLEFUNC(MPSGameLiftClientSystemComponent, HostSessionOnQueue, AZ::ConsoleFunctorFlags::DontReplicate, "Host a new session on queue");

        //! Join an existing game session via the provided session Id
        //! Use console command MPSGameLiftClientSystemComponent.JoinSession <session-id>
        void JoinSession(const AZ::ConsoleCommandContainer& consoleFunctionParameters);
        AZ_CONSOLEFUNC(MPSGameLiftClientSystemComponent, JoinSession, AZ::ConsoleFunctorFlags::DontReplicate, "Join an existing session");

        //! Search for all the existing game sessions on the GameLift fleet specified in the resource mapping configuration file
        //! Use console command MPSGameLiftClientSystemComponent.SearchSessions
        void SearchSessions(const AZ::ConsoleCommandContainer& consoleFunctionParameters);
        AZ_CONSOLEFUNC(MPSGameLiftClientSystemComponent, SearchSessions, AZ::ConsoleFunctorFlags::DontReplicate, "Search for existing sessions");

        //! Leave the current game session
        //! Use console command MPSGameLiftClientSystemComponent.LeaveSession
        void LeaveSession(const AZ::ConsoleCommandContainer& consoleFunctionParameters);
        AZ_CONSOLEFUNC(MPSGameLiftClientSystemComponent, LeaveSession, AZ::ConsoleFunctorFlags::DontReplicate, "Leave the current session");

        //! Start a matchmaking request using the matchmaking configuration specified in the resource mapping configuration file
        //! Use console command MPSGameLiftClientSystemComponent.StartMatchmaking <ticket-id> or MPSGameLiftClientSystemComponent.StartMatchmaking
        void StartMatchmaking(const AZ::ConsoleCommandContainer& consoleFunctionParameters);
        AZ_CONSOLEFUNC(MPSGameLiftClientSystemComponent, StartMatchmaking, AZ::ConsoleFunctorFlags::DontReplicate, "Start a matchmaking request");

        //! Stop the current matchmaking request
        //! Use console command MPSGameLiftClientSystemComponent.StopMatchmaking
        void StopMatchmaking(const AZ::ConsoleCommandContainer& consoleFunctionParameters);
        AZ_CONSOLEFUNC(MPSGameLiftClientSystemComponent, StopMatchmaking, AZ::ConsoleFunctorFlags::DontReplicate, "Stop a matchmaking request");

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
        AZStd::string m_playerId; //!< Unique identifier for the current player inside the game session that can be replaced with any other suitable id.
    };
} // namespace MultiplayerSample#pragma once
