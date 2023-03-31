/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <Components/NetworkMatchComponent.h>
#include <GameState/GameState.h>

namespace MultiplayerSample
{
    class GameStateWaitingForPlayers
        : public GameState::IGameState
        , public PlayerIdentityNotificationBus::Handler
    {
    public:
        AZ_CLASS_ALLOCATOR(GameStateWaitingForPlayers, AZ::SystemAllocator, 0);
        AZ_RTTI(GameStateWaitingForPlayers, "{3c36f38a-8a33-4544-b2cf-8690bd0dca0c}", IGameState);

        explicit GameStateWaitingForPlayers(NetworkMatchComponentController* controller);
        GameStateWaitingForPlayers() = default;

        //! PlayerIdentityNotificationBus
        //! @{
        void OnPlayerActivated(Multiplayer::NetEntityId playerEntity) override;
        //! }@

    private:
        NetworkMatchComponentController* m_controller = nullptr;

        void BeginMatch();
        AZ::ScheduledEvent m_beginMatchEvent{ [this]() 
            {
                BeginMatch(); 
            }, 
            AZ::Name("GameStateWaitingForPlayersBeginMatch") };
    };
}
