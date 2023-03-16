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
#include <AzCore/Math/Random.h>
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

        //! Checks if the current game state allows player action.
        //! For example, in between rounds the player shouldn't be able to move around.
        //! @result true if the player is currently allowed to run, jump, shoot, etc; otherwise false.
        bool IsPlayerActionAllowed() const;

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
        // Possible pre/post names for automatically assigned player gamer name. Upon joining, a player will be given a name e.g. RoboRiot.
        const AZStd::vector<PlayerNameString> AutoAssignedPlayerNamePrefix{ "Robo", "Circuit", "Sparky", "Bolt", "Mech", "Metal", "Junkyard", "Wire", "Steel", "Cog", "Electric", "Circuit", "Rusty", "Byte", "Cyber", "Gizmo", "Sprocket", "Electro", "Plasma", "Automated", "Cybernetic", "Bionic" };
        const AZStd::vector<PlayerNameString> AutoAssignedPlayerNamePostfix{ "Riot", "Crusher", "Sparks", "Bot", "Marauder", "Warrior", "Samurai", "Commando", "Enigma", "Champion", "Renegade", "Brawler", "Crusader", "Gladiator", "Battler", "Heavy", "Miner", "Simulant", "Mecha", "Automata", "Cyborg", "Clunker", "Automat"};

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

        void StartRound();
        void EndRound();

        void HandleRPC_PlayerActivated(AzNetworking::IConnection* invokingConnection, const Multiplayer::NetEntityId& playerEntity) override;
        void HandleRPC_PlayerDeactivated(AzNetworking::IConnection* invokingConnection, const Multiplayer::NetEntityId& playerEntity) override;
#endif

    private:

#if AZ_TRAIT_SERVER
        void RoundTickOnceASecond();
        AZ::ScheduledEvent m_roundTickEvent{[this]()
        {
            RoundTickOnceASecond();
        }, AZ::Name("NetworkMatchComponentController")};

        void RestTickOnceASecond();
        AZ::ScheduledEvent m_restTickEvent{ [this]()
        {
            RestTickOnceASecond();
        }, AZ::Name("NetworkMatchRestClock") };
#endif

        //! List of active players in the match.
        AZStd::vector<Multiplayer::NetEntityId> m_players;

#if AZ_TRAIT_SERVER
        //! A temporary way to assign player identities, such as player names.
        void AssignPlayerIdentity(Multiplayer::NetEntityId playerEntity);
        PlayerNameString GeneratePlayerName();
        int m_nextPlayerId = 1;
        int m_playerNameRandomStartingIndexPrefix = 0;
        int m_playerNameRandomStartingIndexPostfix = 0;

        void RespawnPlayer(Multiplayer::NetEntityId playerEntity, PlayerResetOptions resets);
#endif

        void FindWinner(MatchResultsSummary& results, const AZStd::vector<PlayerState>& potentialWinners);
    };
}
