/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AzCore/Serialization/SerializeContext.h>
#include <Source/Components/Multiplayer/MatchPlayerCoinsComponent.h>

namespace MultiplayerSample
{
    void MatchPlayerCoinsComponent::Reflect(AZ::ReflectContext* context)
    {
        AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context);
        if (serializeContext)
        {
            serializeContext->Class<MatchPlayerCoinsComponent, MatchPlayerCoinsComponentBase>()
                ->Version(1);
        }
        MatchPlayerCoinsComponentBase::Reflect(context);
    }

    void MatchPlayerCoinsComponent::OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        if (IsNetEntityRoleClient())
        {
            MatchPlayerCoinsRequestBus::Handler::BusConnect();
        }
    }

    void MatchPlayerCoinsComponent::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        MatchPlayerCoinsRequestBus::Handler::BusDisconnect();
    }

    AZStd::vector<PlayerCoinState> MatchPlayerCoinsComponent::GetPlayerCoinCounts() const
    {
        AZStd::vector<PlayerCoinState> out;
        // We can return AZStd::array<PlayerCoinState, N> but only if the property is not rewindable, this will support either option.
        const auto& coins = GetCoinsPerPlayerArray();
        for (const auto& state : coins)
        {
            out.push_back(PlayerCoinState(state));
        }

        return out;
    }

    MatchPlayerCoinsComponentController::MatchPlayerCoinsComponentController(MatchPlayerCoinsComponent& parent)
        : MatchPlayerCoinsComponentControllerBase(parent)
    {
    }

    void MatchPlayerCoinsComponentController::OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        PlayerCoinCollectorNotificationBus::Handler::BusConnect();
    }

    void MatchPlayerCoinsComponentController::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        PlayerCoinCollectorNotificationBus::Handler::BusDisconnect();
    }

    void MatchPlayerCoinsComponentController::ResetAllCoins()
    {
        for (int i = 0; i < MultiplayerSample::MaxSupportedPlayers; ++i)
        {
            ModifyCoinsPerPlayer(i).m_coins = 0;
        }
    }

    void MatchPlayerCoinsComponentController::OnPlayerCollectedCoinCountChanged(Multiplayer::NetEntityId playerEntity,
        uint16_t coinsCollected)
    {
        const int stateIndex = GetCoinStateIndex(playerEntity);
        if (stateIndex >= 0)
        {
            ModifyCoinsPerPlayer(stateIndex).m_playerId = playerEntity;
            ModifyCoinsPerPlayer(stateIndex).m_coins = coinsCollected;
        }
    }

    void MatchPlayerCoinsComponentController::OnPlayerCollectorActivated(Multiplayer::NetEntityId playerEntity)
    {
        // Find an empty slot to store this player's state in.
        int32_t stateIndex = 0;
        const int32_t stateCount = aznumeric_cast<int32_t>(GetCoinsPerPlayerArray().size());
        for (; stateIndex < stateCount; ++stateIndex)
        {
            if (GetCoinsPerPlayer(stateIndex).m_playerId == Multiplayer::InvalidNetEntityId)
            {
                break;
            }
        }

        if (stateIndex >= 0 && stateIndex < stateCount)
        {
            ModifyCoinsPerPlayer(stateIndex).m_playerId = playerEntity;
            ModifyCoinsPerPlayer(stateIndex).m_coins = 0;
        }
    }

    void MatchPlayerCoinsComponentController::OnPlayerCollectorDeactivated(Multiplayer::NetEntityId playerEntity)
    {
        const int stateIndex = GetCoinStateIndex(playerEntity);
        if (stateIndex >= 0)
        {
            ModifyCoinsPerPlayer(stateIndex).m_playerId = Multiplayer::InvalidNetEntityId;
            ModifyCoinsPerPlayer(stateIndex).m_coins = 0;
        }
    }

    int MatchPlayerCoinsComponentController::GetCoinStateIndex(Multiplayer::NetEntityId playerEntity) const
    {
        int32_t stateIndex = 0;
        const int32_t stateCount = aznumeric_cast<int32_t>(GetCoinsPerPlayerArray().size());
        for (; stateIndex < stateCount; ++stateIndex)
        {
            if (GetCoinsPerPlayer(stateIndex).m_playerId == playerEntity)
            {
                break;
            }
        }

        if (stateIndex >= 0 && stateIndex < stateCount)
        {
            return stateIndex;
        }

        return -1;
    }
}
