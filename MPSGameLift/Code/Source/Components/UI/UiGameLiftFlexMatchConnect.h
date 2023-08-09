/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <Authorization/AWSCognitoAuthorizationBus.h>
#include <AzCore/Component/Component.h>
#include <MPSGameLift/IRegionalLatencyFinder.h>
#include <Multiplayer/IMultiplayer.h>
#include <Multiplayer/Session/ISessionHandlingRequests.h>

namespace MPSGameLift
{
    /*!
     * \class UiGameLiftFlexMatchConnect
     * \brief An example ui component used for connecting to GameLift using a user-provided JSON string that contains the game-session-id and player-session-id.
    */
    class UiGameLiftFlexMatchConnect
        : public AZ::Component
        , AWSClientAuth::AWSCognitoAuthorizationNotificationBus::Handler
    {
        static constexpr char StatusPlayerAuthInitSuccess[] = "Authorization initialized.";
        static constexpr char StatusPlayerAuthInitFailed[] = "Failed to initialized authorization service.";
        
        static constexpr char StatusAnonPlayerCredentialsReceived[] = "Anonymous player credentials received.";
        static constexpr char StatusAnonPlayerCredentialsFailed[] = "Failed to receive anonymous player credentials.";

        static constexpr char StatusRequestingServerRegionLatencies[] = "Finding server region latencies...";
        static constexpr char StatusLatencyRequestFailed[] = "Failed to reach any server endpoints.\nMake sure you are connected to the internet and try again.";
        static constexpr char StatusLatencyRequestSuccess[] = "Server endpoint latencies:\n%s.";

        static constexpr char StatusMatchmakingTicketReceived[] = "Matchmaking Ticket: %s.";
        static constexpr char StatusMatchmakingMatchFound[] = "Match found!";
        static constexpr char StatusMatchmakingFailedToReceiveTicket[] = "Failed to receive matchmaking ticket.\nAre server fleets running?";
        static constexpr char StatusMatchmakingFailedToReceiveStatusUpdate[] = "Failed to receive status update.\nMake sure you are connected to the internet and try again.";
        static constexpr char StatusMatchmakingTimedOut[] = "Matchmaking timed out!\nReport ticket id to a developer.";

    public:
        AZ_COMPONENT(MPSGameLift::UiGameLiftFlexMatchConnect, "{EFB9D394-8134-400F-B751-42BA81CD08A7}");

        /*
        * Reflects component data into the reflection contexts, including the serialization, edit, and behavior contexts.
        */
        static void Reflect(AZ::ReflectContext* context);

    protected:
        void Activate() override;
        void Deactivate() override;
    
    private:
        // AWSClientAuth::AWSCognitoAuthorizationNotificationBus::Handler overrides...
        void OnRequestAWSCredentialsSuccess(const AWSClientAuth::ClientAuthAWSCredentials& awsCredentials) override;
        void OnRequestAWSCredentialsFail(const AZStd::string& error) override;
        
        // RequestLatenciesCompleteEventHandler...
        void OnRequestLatenciesComplete(const RegionalLatencies& regionLatencies);
        RequestLatenciesCompleteEvent::Handler m_requestLatenciesComplete{ [this](const RegionalLatencies& regionLatencies) { OnRequestLatenciesComplete(regionLatencies); }};

        // Listen for matchmaking events...
        MatchmakingSuccessEvent::Handler m_onMatchmakingSuccess{ [this]()
        {
            PushStatusUpdate(StatusMatchmakingMatchFound);
        } };
        
        MatchmakingFailedEvent::Handler m_onMatchmakingFailed{ [this](MatchMakingFailReason reason)
        {
            switch(reason)
            {
            case MatchMakingFailReason::FailedToReceiveTicket:
                PushStatusFail(StatusMatchmakingFailedToReceiveTicket);
                break;
            case MatchMakingFailReason::FailedToReceiveStatusUpdate:
                PushStatusFail(StatusMatchmakingFailedToReceiveStatusUpdate);
                break;
            case MatchMakingFailReason::TimedOut:
                PushStatusFail(StatusMatchmakingTimedOut);
                break;
            }
        } };

        MatchmakingTicketReceivedEvent::Handler m_onMatchmakingTicketReceived{ [this](AZStd::string ticketId)
        {
            PushStatusUpdate(AZStd::string::format(StatusMatchmakingTicketReceived, ticketId.c_str()));
        } };
        
        // Listen for disconnect events to know if connecting to the host server failed
        void OnConnectToHostFailed();
        Multiplayer::EndpointDisconnectedEvent::Handler m_onConnectToHostFailed{ [this]([[maybe_unused]] Multiplayer::MultiplayerAgentType agent) { OnConnectToHostFailed(); } };
        
        void OnButtonClicked(AZ::EntityId buttonEntityId);

        void PushStatusUpdate(const AZStd::string& statusUpdate);
        void ReplaceStatusUpdate(const AZStd::string& statusUpdate);
        void PushStatusFail(const AZStd::string& reason);
        void RenderStatusText();

        AZ::EntityId m_connectButtonUi;
        AZ::EntityId m_matchmakingStatusTextUi;
        AZ::EntityId m_quitButtonUi;
        AZ::EntityId m_attemptConnectionBlockerUi;
        AZ::EntityId m_connectToHostFailedUi;
        Multiplayer::SessionConnectionConfig m_sessionConnectionConfig;
        AZStd::string m_region;
        
        AZStd::vector<AZStd::string> m_statusUpdates;
    };
} // namespace MultiplayerSample
