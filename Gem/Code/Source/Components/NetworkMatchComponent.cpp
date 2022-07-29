/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <Source/Components/NetworkMatchComponent.h>

namespace MultiplayerSample
{
    NetworkMatchComponentController::NetworkMatchComponentController(NetworkMatchComponent& parent)
        : NetworkMatchComponentControllerBase(parent)
    {
    }

    void NetworkMatchComponentController::OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        SetRoundTime(GetRoundDuration());
        SetRoundNumber(1);
        AZ::TickBus::Handler::BusConnect();
    }

    void NetworkMatchComponentController::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        AZ::TickBus::Handler::BusDisconnect();
    }

    void NetworkMatchComponentController::EndMatch()
    {
        //Signal event to end the match
        ;
    }

    void NetworkMatchComponentController::EndRound()
    {
        uint16_t& roundNumber = ModifyRoundNumber();
        if (roundNumber < GetTotalRounds())
        {
            //Signal event to reset everything
            ++roundNumber;
            SetRoundTime(GetRoundDuration());
        }
        else
        {
            //Signal event to end match
            EndMatch();
        }
    }

    void NetworkMatchComponentController::OnTick([[maybe_unused]] float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time)
    {
        float& roundTime = ModifyRoundTime();
        roundTime -= deltaTime;
        if (roundTime < 0)
        {
            EndRound();
        }
    }

    int NetworkMatchComponentController::GetTickOrder()
    {
        return AZ::TICK_PRE_RENDER;
    }
}
