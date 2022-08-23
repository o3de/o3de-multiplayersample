/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzNetworking/DataStructures/FixedSizeBitset.h>
#include <AzNetworking/Utilities/QuantizedValues.h>
#include <Multiplayer/MultiplayerTypes.h>

namespace MultiplayerSample
{
    constexpr AZStd::string_view WinningCoinCountSetting = "/MultiplayerSample/Settings/WinningCoinCount";

    using StickAxis = AzNetworking::QuantizedValues<1, 1, -1, 1>;
    using MouseAxis = AzNetworking::QuantizedValues<1, 2, -1, 1>;

    //! Various character animation states.
    enum class CharacterAnimState
    {
        Idle,
        Sprinting,
        Crouching,
        Jumping,
        Falling,
        Landing,
        Climbing,
        Aiming,
        Shooting,
        Hit,
        Dying,
        MAX
    };
    using CharacterAnimStateBitset = AzNetworking::FixedSizeBitset<static_cast<AZStd::size_t>(CharacterAnimState::MAX)>;

    enum class Action
    {
        Default,
        Strafing,
        Sprinting,
        Jumping,
        Crouching,
        COUNT = Crouching + 1
    };

    using RoundTimeSec = AzNetworking::QuantizedValues<1, 2, 0, 3600>; // 1 hour max round duration

    static constexpr int MaxSupportedPlayers = 8;

    // Temporary match player state.
    struct PlayerCoinState
    {
        Multiplayer::NetEntityId m_playerId = Multiplayer::InvalidNetEntityId;
        uint16_t m_coins = 0;

        friend bool operator==(const PlayerCoinState& lhs, const PlayerCoinState& rhs)
        {
            return lhs.m_playerId == rhs.m_playerId
                && lhs.m_coins == rhs.m_coins;
        }

        friend bool operator!=(const PlayerCoinState& lhs, const PlayerCoinState& rhs)
        {
            return !(lhs == rhs);
        }

        bool Serialize(AzNetworking::ISerializer& serializer)
        {
            return serializer.Serialize(m_playerId, "PlayerId") && serializer.Serialize(m_coins, "Coins");
        }
    };

    using PlayerNameString = AZStd::fixed_string<50>;

    struct PlayerState
    {
        PlayerNameString m_playerName;
        uint32_t m_score = 0;          // coins collected
        uint8_t m_remainingShield = 0; // % of shield left, max of ~200% allowed for buffs
        bool operator!=(const PlayerState& rhs) const;
        bool Serialize(AzNetworking::ISerializer& serializer);
    };

    inline bool PlayerState::Serialize(AzNetworking::ISerializer& serializer)
    {
        return serializer.Serialize(m_playerName, "playerName")
            && serializer.Serialize(m_score, "score")
            && serializer.Serialize(m_remainingShield, "remainingShield");
    }

    inline bool PlayerState::operator!=(const PlayerState& rhs) const
    {
        return m_playerName != rhs.m_playerName
            || m_score != rhs.m_score
            || m_remainingShield != rhs.m_remainingShield;
    }

    struct MatchResultsSummary
    {
        PlayerNameString m_winningPlayerName;
        AZStd::vector<PlayerState> m_playerStates;
        bool operator!=(const MatchResultsSummary& rhs) const;
        bool Serialize(AzNetworking::ISerializer& serializer);
    };

    inline bool MatchResultsSummary::Serialize(AzNetworking::ISerializer& serializer)
    {
        return serializer.Serialize(m_winningPlayerName, "winningPlayerName")
            && serializer.Serialize(m_playerStates, "playerStates");
    }

    inline bool MatchResultsSummary::operator!=(const MatchResultsSummary& rhs) const
    {
        return m_winningPlayerName != rhs.m_winningPlayerName;
    }

    struct PlayerResetOptions
    {
        bool m_resetArmor = false;
        uint16_t m_coinPenalty = 0;
        bool Serialize(AzNetworking::ISerializer& serializer);
    };

    inline bool PlayerResetOptions::Serialize(AzNetworking::ISerializer& serializer)
    {
        return serializer.Serialize(m_resetArmor, "resetArmor")
            && serializer.Serialize(m_coinPenalty, "coinPenalty");
    }
}

namespace AZ
{
    AZ_TYPE_INFO_SPECIALIZE(MultiplayerSample::CharacterAnimState, "{2DC36B4D-3B14-45A8-911A-60F8732F6A88}");
    AZ_TYPE_INFO_SPECIALIZE(MultiplayerSample::Action, "{1BFDEBD3-ED36-465D-BFA0-9160CFB24F37}");
}
