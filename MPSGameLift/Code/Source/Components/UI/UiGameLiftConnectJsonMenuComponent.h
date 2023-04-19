/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/Component/Component.h>
#include <Multiplayer/IMultiplayer.h>

#include <Request/AWSGameLiftJoinSessionRequest.h>
#include <Multiplayer/Session/ISessionRequests.h>

namespace MPSGameLift
{
    /*!
     * \class UiGameLiftConnectJsonMenuComponent
     * \brief Ui component used for connecting to GameLift using a JSON string containing the fleet-id, game-session, player-session, etc.
    */
    class UiGameLiftConnectJsonMenuComponent
        : public AZ::Component,
          Multiplayer::SessionAsyncRequestNotificationBus::Handler
    {
    public:
        AZ_COMPONENT(MPSGameLift::UiGameLiftConnectJsonMenuComponent, "{328C97C3-D4BC-4A07-94F1-E1462908FC7A}");

        /*
        * Reflects component data into the reflection contexts, including the serialization, edit, and behavior contexts.
        */
        static void Reflect(AZ::ReflectContext* context);

    protected:
        void Activate() override;
        void Deactivate() override;

    private:
        //! Multiplayer::SessionAsyncRequestNotificationBus overrides...
        void OnCreateSessionAsyncComplete(const AZStd::string& ) override {}
        void OnSearchSessionsAsyncComplete(const Multiplayer::SearchSessionsResponse& ) override {}
        void OnJoinSessionAsyncComplete(bool joinSessionsResponse) override;
        void OnLeaveSessionAsyncComplete() override {}
        
        // Listen for disconnect events to know if connecting to the host server failed
        void OnConnectToHostFailed();
        Multiplayer::EndpointDisconnectedEvent::Handler m_onConnectToHostFailed{[this]([[maybe_unused]] Multiplayer::MultiplayerAgentType agent) { OnConnectToHostFailed(); }};

        void OnButtonClicked(AZ::EntityId buttonEntityId) const;
        void OnJSONChanged(const AZStd::string& gameLiftJsonString);

        AZ::EntityId m_connectButtonUi;
        AZ::EntityId m_quitButtonUi;
        AZ::EntityId m_jsonInputUi;
        AZ::EntityId m_attemptConnectionBlockerUi;
        AZ::EntityId m_connectToHostFailedUi;
        AZ::EntityId m_jsonParseFailTextUi;
        AWSGameLift::AWSGameLiftJoinSessionRequest m_request;
    };
} // namespace MultiplayerSample