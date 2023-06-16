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

#include <Multiplayer/Session/ISessionHandlingRequests.h>

namespace MPSGameLift
{
    /*!
     * \class UiGameLiftConnectWithPlayerSessionData
     * \brief An example ui component used for connecting to GameLift using a user-provided JSON string that contains the game-session-id and player-session-id.
    */
    class UiGameLiftConnectWithPlayerSessionData
        : public AZ::Component
    {
    public:
        AZ_COMPONENT(MPSGameLift::UiGameLiftConnectWithPlayerSessionData, "{328C97C3-D4BC-4A07-94F1-E1462908FC7A}");

        /*
        * Reflects component data into the reflection contexts, including the serialization, edit, and behavior contexts.
        */
        static void Reflect(AZ::ReflectContext* context);

    protected:
        void Activate() override;
        void Deactivate() override;

    private:        
        // Listen for disconnect events to know if connecting to the host server failed
        void OnConnectToHostFailed();
        Multiplayer::EndpointDisconnectedEvent::Handler m_onConnectToHostFailed{[this]([[maybe_unused]] Multiplayer::MultiplayerAgentType agent) { OnConnectToHostFailed(); }};

        void OnButtonClicked(AZ::EntityId buttonEntityId) const;
        void OnJSONChanged(const AZStd::string& gameLiftJsonString);

        AZ::EntityId m_connectButtonUi;
        AZ::EntityId m_quitButtonUi;
        AZ::EntityId m_playerSessionDataJsonInputUi;
        AZ::EntityId m_attemptConnectionBlockerUi;
        AZ::EntityId m_connectToHostFailedUi;
        AZ::EntityId m_jsonParseFailTextUi;
        Multiplayer::SessionConnectionConfig m_sessionConnectionConfig;
        AZStd::string m_region;
    };
} // namespace MultiplayerSample
