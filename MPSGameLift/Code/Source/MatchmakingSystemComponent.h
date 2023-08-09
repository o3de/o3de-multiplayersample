/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 * 
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/Component/Component.h>
#include <AzCore/Console/ILogger.h>
#include <AzCore/EBus/ScheduledEvent.h>
#include <MPSGameLift/IMatchmaking.h>

namespace MPSGameLift
{
    class MatchmakingSystemComponent final
        : public AZ::Component
        , public IMatchmaking
    {
    public:
        static constexpr float MatchRequestTimeoutSeconds = 60.0f;

        AZ_COMPONENT(MatchmakingSystemComponent, "{BF5F9343-63B5-4703-89ED-9CDBF4FE6004}");
        static void Reflect(AZ::ReflectContext* context);
        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);

        // IMatchmaking overrides...
        bool RequestMatch(const RegionalLatencies& regionalLatencies) override;
        AZStd::string GetTicketId() const override { return m_ticketId; }
        void AddMatchmakingTicketReceivedEventHandler(MatchmakingTicketReceivedEvent::Handler& handler) override;
        void AddMatchmakingSuccessEventHandler(MatchmakingSuccessEvent::Handler& handler) override;
        void AddMatchmakingFailedEventHandler(MatchmakingFailedEvent::Handler& handler) override;

     protected:
        void Activate() override;
        void Deactivate() override;

        // Request matchmaking status.
        // After requesting a match the client polls every few seconds for a successful match.
        // The HTTP response contains a PlayerSessionId which is either "NotPlacedYet" or a proper UUID.
        // A successful response will contain all the information required for the client to join the server;
        //     Example: the DNS (or IP address), port, and player session id.
        void RequestMatchStatus();

    private:
        AZStd::string m_ticketId;
        AZ::ScheduledEvent m_requestMatchStatusEvent = AZ::ScheduledEvent([this] { this->RequestMatchStatus(); }, AZ::Name("MPS Request Match Status"));
        AZ::ScheduledEvent m_requestMatchTimeoutEvent = AZ::ScheduledEvent([this] 
            { 
                AZLOG_ERROR("MatchmakingSystemComponent: Match request timed out! " 
                    "Matches should start even if only 1 player is found; the backend might not be configured properly.");
                m_requestMatchStatusEvent.RemoveFromQueue();
                m_matchRequestTimeout = true;
                m_matchmakingFailedEvent.Signal(MatchMakingFailReason::TimedOut);
            }
        , AZ::Name("MPS Request Match Timeout"));
        
        bool m_matchRequestTimeout = false;

        // Matchmaking Events
        MatchmakingTicketReceivedEvent m_matchmakingTicketReceivedEvent;
        MatchmakingSuccessEvent m_matchmakingSuccessEvent;
        MatchmakingFailedEvent m_matchmakingFailedEvent;

    };
}
