/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <PlayerIdentityBus.h>
#include <Source/Components/NetworkHealthComponent.h>
#include <Source/Components/Multiplayer/PlayerCoinCollectorComponent.h>
#include <Source/Components/Multiplayer/PlayerIdentityComponent.h>

namespace MultiplayerSample
{
    void PlayerIdentityComponent::Reflect(AZ::ReflectContext* context)
    {
        AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context);
        if (serializeContext)
        {
            serializeContext->Class<PlayerIdentityComponent, PlayerIdentityComponentBase>()
                ->Version(1);
        }
        PlayerIdentityComponentBase::Reflect(context);
    }

    void PlayerIdentityComponent::OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
    }

    void PlayerIdentityComponent::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
    }

    void PlayerIdentityComponent::AssignPlayerName(const PlayerNameString& newPlayerName)
    {
        if (IsNetEntityRoleServer())
        {
            RPC_AssignPlayerName(newPlayerName);
        }
        else
        {
            static_cast<PlayerIdentityComponentController*>(GetController())->SetPlayerName(newPlayerName);
        }
    }


    PlayerIdentityComponentController::PlayerIdentityComponentController(PlayerIdentityComponent& parent)
        : PlayerIdentityComponentControllerBase(parent)
    {
    }

    void PlayerIdentityComponentController::OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        if (IsNetEntityRoleAuthority())
        {
            PlayerIdentityNotificationBus::Broadcast(&PlayerIdentityNotificationBus::Events::OnPlayerActivated, GetNetEntityId());
        }

        if (IsNetEntityRoleAutonomous())
        {
            PlayerIdentityRequestBus::Handler::BusConnect();
        }
    }

    void PlayerIdentityComponentController::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        if (IsNetEntityRoleAuthority())
        {
            PlayerIdentityNotificationBus::Broadcast(&PlayerIdentityNotificationBus::Events::OnPlayerDeactivated, GetNetEntityId());
        }

        PlayerIdentityRequestBus::Handler::BusDisconnect();
    }

    void PlayerIdentityComponentController::HandleRPC_AssignPlayerName([[maybe_unused]] AzNetworking::IConnection* invokingConnection,
        const PlayerNameString& newPlayerName)
    {
        SetPlayerName(newPlayerName);
    }

    void PlayerIdentityComponentController::HandleRPC_ResetPlayerState([[maybe_unused]] AzNetworking::IConnection* invokingConnection, const PlayerResetOptions& resetOptions)
    {
        if (resetOptions.m_resetArmor)
        {
            GetNetworkHealthComponentController()->SetHealth(GetNetworkHealthComponentController()->GetMaxHealth());
        }
        auto currentCoins = GetPlayerCoinCollectorComponentController()->GetCoinsCollected();
        float coinsToDeduct = currentCoins * (resetOptions.m_coinPenalty * 0.01f);

        GetPlayerCoinCollectorComponentController()->SetCoinsCollected(currentCoins - aznumeric_cast<uint16_t>(coinsToDeduct));
        PlayerIdentityComponentControllerBase::HandleRPC_ResetPlayerState(invokingConnection, resetOptions);
    }

    const char* PlayerIdentityComponentController::GetPlayerIdentityName()
    {
        return GetParent().GetPlayerName().c_str();
    }
}
