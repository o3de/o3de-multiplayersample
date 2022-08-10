/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <Source/Components/NetworkMatchComponent.h>
#include <MultiplayerSampleTypes.h>
#include <UiGameOverBus.h>

namespace MultiplayerSample
{
    void NetworkMatchComponent::Reflect(AZ::ReflectContext* context)
    {
        AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context);
        if (serializeContext)
        {
            serializeContext->Class<NetworkMatchComponent, NetworkMatchComponentBase>()
                ->Version(1);
        }
        NetworkMatchComponentBase::Reflect(context);
    }

    void NetworkMatchComponent::HandleRPC_EndMatch(
        [[maybe_unused]] AzNetworking::IConnection* invokingConnection, [[maybe_unused]] const MatchResultsSummary& results)
    {
        if (IsNetEntityRoleClient())
        {
            UiGameOverBus::Broadcast(&UiGameOverBus::Events::SetGameOverScreenEnabled, true);
            UiGameOverBus::Broadcast(&UiGameOverBus::Events::DisplayResults, results);
        }
    }

    // Controller methods

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

        // TODO: continuously populate player state base on coins, etc.
        PlayerState jackState = PlayerState{ "Jack", 25, 80 };
        PlayerState allieState = PlayerState{ "Allie", 23, 70 };
        PlayerState olexState = PlayerState{ "Olex", 5, 42 };

        SetPlayerStates(0, jackState);
        SetPlayerStates(1, allieState);
        SetPlayerStates(2, olexState);

        // TODO: formulate and send real results
        RPC_EndMatch(MatchResultsSummary{ "Jack", {jackState, allieState, olexState} });
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
