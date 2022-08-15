/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <PlayerIdentityBus.h>
#include <AzCore/EBus/ScheduledEvent.h>
#include <Source/AutoGen/NetworkMatchComponent.AutoComponent.h>

namespace MultiplayerSample
{
    class NetworkMatchComponent
        : public NetworkMatchComponentBase
        , public PlayerIdentityNotificationBus::Handler
    {
    public:
        AZ_MULTIPLAYER_COMPONENT(MultiplayerSample::NetworkMatchComponent, s_networkMatchComponentConcreteUuid, MultiplayerSample::NetworkMatchComponentBase);

        static void Reflect(AZ::ReflectContext* context);
        
        void OnActivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
        void OnDeactivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;

        //! PlayerIdentityNotificationBus
        //! @{
        void OnPlayerActivated(Multiplayer::NetEntityId playerEntity) override;
        void OnPlayerDeactivated(Multiplayer::NetEntityId playerEntity) override;
        //! }@

        void HandleRPC_EndMatch(
            AzNetworking::IConnection* invokingConnection, const MatchResultsSummary& results) override;
    };

    class NetworkMatchComponentController
        : public NetworkMatchComponentControllerBase
    {
    public:
        explicit NetworkMatchComponentController(NetworkMatchComponent& parent);

        void OnActivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
        void OnDeactivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;

        void EndMatch();
        void EndRound();

        void HandleRPC_PlayerActivated(AzNetworking::IConnection* invokingConnection, const Multiplayer::NetEntityId& playerEntity) override;
        void HandleRPC_PlayerDeactivated(AzNetworking::IConnection* invokingConnection, const Multiplayer::NetEntityId& playerEntity) override;

    private:
        void RoundTickOnceASecond();
        AZ::ScheduledEvent m_roundTickEvent{[this]()
        {
            RoundTickOnceASecond();
        }, AZ::Name("NetworkMatchComponentController")};

        //! List of active players in the match.
        AZStd::vector<Multiplayer::NetEntityId> m_players;

        //! A temporary way to assign player identities, such as player names.
        void AssignPlayerIdentity(Multiplayer::NetEntityId playerEntity);
        int m_nextPlayerId = 1;
    };
}
