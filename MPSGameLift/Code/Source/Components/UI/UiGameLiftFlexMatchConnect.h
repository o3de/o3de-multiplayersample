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
#include <MPSGameLift/IRegionalLatencyFinder.h>

#include <Multiplayer/Session/ISessionHandlingRequests.h>

namespace MPSGameLift
{
    /*!
     * \class UiGameLiftFlexMatchConnect
     * \brief An example ui component used for connecting to GameLift using a user-provided JSON string that contains the game-session-id and player-session-id.
    */
    class UiGameLiftFlexMatchConnect
        : public AZ::Component
    {
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

        // RegionalLatencyFinderNotificationBus::Handler overrides...
        void OnRequestLatenciesComplete(const RegionalLatencies& regionLatencies);

        // Listen for disconnect events to know if connecting to the host server failed
        void OnConnectToHostFailed();
        Multiplayer::EndpointDisconnectedEvent::Handler m_onConnectToHostFailed{[this]([[maybe_unused]] Multiplayer::MultiplayerAgentType agent) { OnConnectToHostFailed(); }};

        RequestLatenciesCompleteEvent::Handler m_requestLatenciesComplete{ [this](const RegionalLatencies& regionLatencies) { OnRequestLatenciesComplete(regionLatencies); }};

        void OnButtonClicked(AZ::EntityId buttonEntityId) const;

        AZ::EntityId m_connectButtonUi;
        AZ::EntityId m_matchmakingStatusTextUi;
        AZ::EntityId m_quitButtonUi;
        AZ::EntityId m_attemptConnectionBlockerUi;
        AZ::EntityId m_connectToHostFailedUi;
        Multiplayer::SessionConnectionConfig m_sessionConnectionConfig;
        AZStd::string m_region;
    };
} // namespace MultiplayerSample
