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

        // Tick once a second, this way we can keep the time as an 2 byte integer instead of a float.
        m_roundTickEvent.Enqueue(AZ::TimeMs{ 1000 }, true);
    }

    void NetworkMatchComponentController::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        m_roundTickEvent.RemoveFromQueue();
    }

    void NetworkMatchComponentController::EndMatch()
    {
        //Signal event to end the match
        m_roundTickEvent.RemoveFromQueue();
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

    void NetworkMatchComponentController::RoundTickOnceASecond()
    {
        int16_t& roundTime = ModifyRoundTime();
        roundTime--; // m_roundTickEvent is configured to tick once a second
        if (roundTime <= 0)
        {
            EndRound();
        }
    }
}
