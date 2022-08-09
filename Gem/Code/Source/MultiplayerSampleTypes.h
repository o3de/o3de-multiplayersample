/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 * 
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzNetworking/Utilities/QuantizedValues.h>
#include <AzNetworking/DataStructures/FixedSizeBitset.h>

namespace MultiplayerSample
{
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
}

namespace AZ
{
    AZ_TYPE_INFO_SPECIALIZE(MultiplayerSample::CharacterAnimState, "{2DC36B4D-3B14-45A8-911A-60F8732F6A88}");
    AZ_TYPE_INFO_SPECIALIZE(MultiplayerSample::Action, "{1BFDEBD3-ED36-465D-BFA0-9160CFB24F37}");
}
