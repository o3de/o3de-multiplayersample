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
        SetRoundTime(RoundTimeSec{ GetRoundDuration() });
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
            SetRoundTime(RoundTimeSec{ GetRoundDuration() });
        }
        else
        {
            //Signal event to end match
            EndMatch();
        }
    }

    void NetworkMatchComponentController::RoundTickOnceASecond()
    {
        // m_roundTickEvent is configured to tick once a second
        SetRoundTime(RoundTimeSec(GetRoundTime() - 1.f));
        
        if (GetRoundTime() <= RoundTimeSec(0.f))
        {
            EndRound();
        }
    }
}
