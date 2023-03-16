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
    class GameStatePreparingMatch
        : public GameState::IGameState
    {
    public:
        AZ_CLASS_ALLOCATOR(GameStatePreparingMatch, AZ::SystemAllocator, 0);
        AZ_RTTI(GameStatePreparingMatch, "{cb985ac1-ad3a-41d9-b437-d29ca6e5a6d1}", IGameState);

        explicit GameStatePreparingMatch(NetworkMatchComponentController* controller);
        GameStatePreparingMatch() = default;

        //! GameState::IGameState overrides ...
        //! @{
        void OnEnter() override;
        void OnExit() override;
        //! }@

    private:
        NetworkMatchComponentController* m_controller = nullptr;

        AZ::TimeMs m_preparationTime = AZ::Time::ZeroTimeMs;

        void OnPreparingMatchTick();
        AZ::ScheduledEvent m_preparingEvent{ [this]()
        {
            OnPreparingMatchTick();
        }, AZ::Name("GameStatePreparingMatch") };
    };
}
