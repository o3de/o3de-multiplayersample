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
    class GameStateMatchInProgress
        : public GameState::IGameState
    {
    public:
        AZ_CLASS_ALLOCATOR(GameStateMatchInProgress, AZ::SystemAllocator, 0);
        AZ_RTTI(GameStateMatchInProgress, "{50beb9f9-b71a-47a4-b4c9-7237518ddb24}", IGameState);

        explicit GameStateMatchInProgress(NetworkMatchComponentController* controller);
        GameStateMatchInProgress() = default;

        //! GameState::IGameState overrides ...
        //! @{
        void OnEnter() override;
        void OnExit() override;
        //! }@

    private:
        NetworkMatchComponentController* m_controller = nullptr;

        void OnRoundChanged(AZ::u16 round);
        AZ::Event<AZ::u16>::Handler m_roundChangedHandler{[this](AZ::u16 round)
        {
            OnRoundChanged(round);
        }};
    };
}
