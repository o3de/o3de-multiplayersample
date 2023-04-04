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
    class GameStateMatchEnded
        : public GameState::IGameState
    {
    public:
        AZ_CLASS_ALLOCATOR(GameStateMatchEnded, AZ::SystemAllocator, 0);
        AZ_RTTI(GameStateMatchEnded, "{57dea8df-1286-4f65-ae4f-d4aab858428b}", IGameState);

        explicit GameStateMatchEnded(NetworkMatchComponentController* controller);
        GameStateMatchEnded() = default;

        //! GameState::IGameState overrides ...
        //! @{
        void OnEnter() override;
        void OnExit() override;
        //! }@

    private:
        NetworkMatchComponentController* m_controller = nullptr;

        void OnFinishedMatch();
        AZ::ScheduledEvent m_finishingEvent{ [this]()
        {
            OnFinishedMatch();
        }, AZ::Name("GameStateMatchEnded") };
    };
}
