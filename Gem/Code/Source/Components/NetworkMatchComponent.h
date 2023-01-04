/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <PlayerIdentityBus.h>
#include <PlayerMatchLifecycleBus.h>
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

#if AZ_TRAIT_SERVER
        //! PlayerIdentityNotificationBus
        //! @{
        void OnPlayerActivated(Multiplayer::NetEntityId playerEntity) override;
        void OnPlayerDeactivated(Multiplayer::NetEntityId playerEntity) override;
        //! }@
#endif

#if AZ_TRAIT_CLIENT
        void HandleRPC_EndMatch(
            AzNetworking::IConnection* invokingConnection, const MatchResultsSummary& results) override;
#endif
    };

    class NetworkMatchComponentController
        : public NetworkMatchComponentControllerBase
        , public PlayerMatchLifecycleBus::Handler
    {
    public:
        explicit NetworkMatchComponentController(NetworkMatchComponent& parent);

        void OnActivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
        void OnDeactivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;

        //! PlayerMatchLifecycleBus overrides
        //! @{
        void OnPlayerArmorZero(Multiplayer::NetEntityId playerEntity) override;
        //! )@

#if AZ_TRAIT_SERVER
        void StartMatch();

        void EndMatch();
        void EndRound();

        void HandleRPC_PlayerActivated(AzNetworking::IConnection* invokingConnection, const Multiplayer::NetEntityId& playerEntity) override;
        void HandleRPC_PlayerDeactivated(AzNetworking::IConnection* invokingConnection, const Multiplayer::NetEntityId& playerEntity) override;
#endif

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

#if AZ_TRAIT_SERVER
        void RespawnPlayer(Multiplayer::NetEntityId playerEntity, PlayerResetOptions resets);
#endif

        void FindWinner(MatchResultsSummary& results, const AZStd::vector<PlayerState>& potentialWinners);
    };
}
