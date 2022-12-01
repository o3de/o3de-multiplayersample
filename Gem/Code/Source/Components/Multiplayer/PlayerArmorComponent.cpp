/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <GameplayEffectsNotificationBus.h>
#include <PlayerMatchLifecycleBus.h>
#include <UiPlayerArmorBus.h>
#include <AzCore/Component/TransformBus.h>
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
        m_previousArmorAmount = GetNetworkHealthComponentController()->GetParent().GetHealth();
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
        if (armor > m_previousArmorAmount)
        {
            GameplayEffectsNotificationBus::Broadcast(&GameplayEffectsNotificationBus::Events::OnPositionalEffect,
                SoundEffect::ArmorMend, GetEntity()->GetTransform()->GetWorldTranslation());
        }
        else if (armor < m_previousArmorAmount)
        {
            const float halfArmor = GetNetworkHealthComponentController()->GetParent().GetMaxHealth() / 2.f;
            if (armor < halfArmor && m_previousArmorAmount > halfArmor)
            {
                // Armor breaking defined as going below 50% of armor
                GameplayEffectsNotificationBus::Broadcast(&GameplayEffectsNotificationBus::Events::OnPositionalEffect,
                    SoundEffect::ArmorBreaking, GetEntity()->GetTransform()->GetWorldTranslation());
            }

            GameplayEffectsNotificationBus::Broadcast(&GameplayEffectsNotificationBus::Events::OnPositionalEffect,
                SoundEffect::PlayerOuch, GetEntity()->GetTransform()->GetWorldTranslation());
        }
        m_previousArmorAmount = armor;

        UiPlayerArmorNotificationBus::Broadcast(&UiPlayerArmorNotificationBus::Events::OnPlayerArmorChanged, armor, GetStartingArmor());
        if (armor <= 0)
        {
            PlayerMatchLifecycleBus::Broadcast(&PlayerMatchLifecycleNotifications::OnPlayerArmorZero, GetNetEntityId());
        }
    }
}
