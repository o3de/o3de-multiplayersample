/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <UiPlayerArmorBus.h>
#include <PlayerMatchLifecycleBus.h>
#include <Components/NetworkHealthComponent.h>
#include <Source/Components/Multiplayer/PlayerArmorComponent.h>

namespace MultiplayerSample
{
    PlayerArmorComponentController::PlayerArmorComponentController(PlayerArmorComponent& parent)
        : PlayerArmorComponentControllerBase(parent)
    {
    }

    void PlayerArmorComponentController::OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        GetNetworkHealthComponentController()->GetParent().HealthAddEvent(m_changedHandler);

        if (IsNetEntityRoleAuthority())
        {
            GetNetworkHealthComponentController()->SetHealth(GetStartingArmor());
        }
    }

    void PlayerArmorComponentController::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        m_changedHandler.Disconnect();
    }

    void PlayerArmorComponentController::OnAmountChanged(float armor)
    {
        UiPlayerArmorNotificationBus::Broadcast(&UiPlayerArmorNotificationBus::Events::OnPlayerArmorChanged, armor, GetStartingArmor());
        if (armor <= 0)
        {
            PlayerMatchLifecycleBus::Broadcast(&PlayerMatchLifecycleNotifications::OnPlayerArmorZero, GetNetEntityId());
        }
    }
}
