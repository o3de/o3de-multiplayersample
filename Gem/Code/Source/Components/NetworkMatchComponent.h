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
    AZ_ENUM_CLASS(AllowedPlayerActions,
        None,
        RotationOnly,
        All
    );

    class INetworkMatch
    {
    public:
        AZ_RTTI(INetworkMatch, "{2EBAF2B1-76E9-4FC8-82DF-BD63FFC372BF}");

        INetworkMatch() = default;
        virtual ~INetworkMatch() = default;

        //! Returns which player actions (if any) the current game state allows.
        //! For example, in between rounds the player shouldn't be able to move around, but they can still rotate.
        //! @result The type of player actions that are allowed 
        virtual AllowedPlayerActions PlayerActionsAllowed() const = 0;

        //! Returns the time in seconds until the current round ends.
        //! @result the time in seconds until the current round ends
        virtual float GetRoundTimeRemainingSec() const = 0;

        //! Returns the total time in seconds until a round ends.
        //! @result the total time in seconds until a round ends
        virtual float GetTotalRoundTimeSec() const = 0;

        //! Returns current the round number.
        //! @result current the round number
        virtual int32_t GetCurrentRoundNumber() const = 0;

        //! Returns the total number of rounds before a game ends.
        //! @result the total number of rounds before a game ends
        virtual int32_t GetTotalRoundCount() const = 0;

        //! Returns the current count of active players.
        //! @result the current count of active players
        virtual int32_t GetTotalPlayerCount() const = 0;

        //! Returns the time the first match begins
        //! Host Time is the time in milliseconds since the host server application has started.
        //! @result the time the first match begins
        virtual AZ::TimeMs GetMatchStartHostTime() const = 0;

        //! Adds an event handler to the round number AZ::Event
        //! @param handler the handler to add the the requested component event
        virtual void AddRoundNumberEventHandler(AZ::Event<uint16_t>::Handler& handler) = 0;

        //! Adds an event handler to the round time remaining AZ::Event
        //! @param handler the handler to add the the requested component event
        virtual void AddRoundTimeRemainingEventHandler(AZ::Event<RoundTimeSec>::Handler& handler) = 0;

        //! Adds an event handler to the round number rest remaining AZ::Event
        //! @param handler the handler to add the the requested component event
        virtual void AddRoundRestTimeRemainingEventHandler(AZ::Event<RoundTimeSec>::Handler& handler) = 0;

        //! Adds an event handler to capture the time the first match is set to begin.
        //! @param handler the handler to add the the requested component event
        virtual void AddFirstMatchStartHostTime(AZ::Event<AZ::TimeMs>::Handler& handler) = 0;
    };


    //! Script-bind for the INetworkMatch interface
    class NetworkMatchComponentRequests
        : public AZ::EBusTraits
    {
    public:
        static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
    };
    using NetworkMatchComponentRequestBus = AZ::EBus<INetworkMatch, NetworkMatchComponentRequests>;

    class NetworkMatchComponent
        : public NetworkMatchComponentBase
        , public NetworkMatchComponentRequestBus::Handler
        , public PlayerIdentityNotificationBus::Handler
    {
    public:
        AZ_MULTIPLAYER_COMPONENT(MultiplayerSample::NetworkMatchComponent, s_networkMatchComponentConcreteUuid, MultiplayerSample::NetworkMatchComponentBase);

        static void Reflect(AZ::ReflectContext* context);

        void OnActivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
        void OnDeactivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;

        //! INetworkMatch interface
        //! @{
        AllowedPlayerActions PlayerActionsAllowed() const override;
        float GetRoundTimeRemainingSec() const override;
        float GetTotalRoundTimeSec() const override;
        int32_t GetCurrentRoundNumber() const override;
        int32_t GetTotalRoundCount() const override;
        int32_t GetTotalPlayerCount() const override;
        AZ::TimeMs GetMatchStartHostTime() const override;
        void AddRoundNumberEventHandler(AZ::Event<uint16_t>::Handler& handler) override;
        void AddRoundTimeRemainingEventHandler(AZ::Event<RoundTimeSec>::Handler& handler) override;
        void AddRoundRestTimeRemainingEventHandler(AZ::Event<RoundTimeSec>::Handler& handler) override;
        void AddFirstMatchStartHostTime(AZ::Event<AZ::TimeMs>::Handler& handler) override;
        //! @}

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

namespace AZ
{
    AZ_TYPE_INFO_SPECIALIZE(MultiplayerSample::AllowedPlayerActions, "{D8EB0533-D50C-4C04-B462-BA0BD1607FA8}");
} // namespace AZ
