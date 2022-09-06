/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/Asset/AssetSerializer.h>
#include <AzFramework/Spawnable/Spawnable.h>
#include <AzNetworking/DataStructures/FixedSizeBitset.h>
#include <AzNetworking/Utilities/QuantizedValues.h>
#include <Multiplayer/MultiplayerTypes.h>

namespace MultiplayerSample
{
    constexpr AZStd::string_view WinningCoinCountSetting = "/MultiplayerSample/Settings/WinningCoinCount";
    constexpr AZStd::string_view KnockbackDistanceByEnergyBallSetting = "/MultiplayerSample/Settings/EnergyBall/KnockbackDistanceMeters";
    constexpr AZStd::string_view EnergyBallSpeedSetting = "/MultiplayerSample/Settings/EnergyBall/Speed";
    constexpr AZStd::string_view EnergyBallArmorDamageSetting = "/MultiplayerSample/Settings/EnergyBall/ArmorDamage";
    constexpr AZStd::string_view EnergyCannonFiringPeriodSetting = "/MultiplayerSample/Settings/EnergyCannon/FiringPeriodMilliseconds";

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
        uint8_t m_remainingArmor = 0;
        bool operator!=(const PlayerState& rhs) const;
        bool Serialize(AzNetworking::ISerializer& serializer);
    };

    inline bool PlayerState::Serialize(AzNetworking::ISerializer& serializer)
    {
        return serializer.Serialize(m_playerName, "Name")
            && serializer.Serialize(m_score, "Score")
            && serializer.Serialize(m_remainingArmor, "Armor");
    }

    inline bool PlayerState::operator!=(const PlayerState& rhs) const
    {
        return m_playerName != rhs.m_playerName
            || m_score != rhs.m_score
            || m_remainingArmor != rhs.m_remainingArmor;
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

    //! Defines a gem type with an asset and a tag name.
    class GemSpawnable
    {
    public:
        AZ_RTTI(GemSpawnable, "{393888AE-F158-40CB-9AA0-46767BDA5694}");
        virtual ~GemSpawnable() = default;

        static void Reflect(AZ::ReflectContext* context);

        AZStd::string m_tag;
        AzFramework::SpawnableAsset m_gemAsset;
    };

    using GemSpawnableVector = AZStd::vector<GemSpawnable>;

    //! Defines a weighted chance for a gem type to spawn in a given round.
    class GemWeightChance
    {
    public:
        AZ_RTTI(GemWeightChance, "{B9B5C6A7-895D-407A-90B7-26FB10C6DA22}");
        virtual ~GemWeightChance() = default;

        static void Reflect(AZ::ReflectContext* context);

        AZStd::string m_tag;
        float m_weight = 1.f;
    };

    //! Defines chances for gem types to spawn in a given round.
    class RoundSpawnTable
    {
    public:
        AZ_RTTI(RoundSpawnTable, "{3AB30950-F71A-4F96-9BB0-C1C44B2A53CC}");
        virtual ~RoundSpawnTable() = default;

        static void Reflect(AZ::ReflectContext* context);

        AZStd::vector<GemWeightChance> m_gemWeights;
    };

    using RoundSpawnTableVector = AZStd::vector<RoundSpawnTable>;
}

namespace AZ
{
    AZ_TYPE_INFO_SPECIALIZE(MultiplayerSample::CharacterAnimState, "{2DC36B4D-3B14-45A8-911A-60F8732F6A88}");
    AZ_TYPE_INFO_SPECIALIZE(MultiplayerSample::Action, "{1BFDEBD3-ED36-465D-BFA0-9160CFB24F37}");
}
