/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <Source/Components/NetworkHealthComponent.h>

namespace MultiplayerSample
{
    void NetworkHealthComponent::NetworkHealthComponent::Reflect(AZ::ReflectContext* context)
    {
        AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context);
        if (serializeContext)
        {
            serializeContext->Class<NetworkHealthComponent, NetworkHealthComponentBase>()
                ->Version(1);
        }
        NetworkHealthComponentBase::Reflect(context);
    }

    NetworkHealthComponent::NetworkHealthComponent()
        : m_healthEventHandler([this](const uint8_t& health) { OnHealthChangedEvent(health); })
    {
        ;
    }

    void NetworkHealthComponent::OnInit()
    {
        ;
    }

    void NetworkHealthComponent::OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        HealthAddEvent(m_healthEventHandler);
    }

    void NetworkHealthComponent::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        ;
    }

    void NetworkHealthComponent::SetHealth(uint8_t updatedHealth)
    {
        updatedHealth = AZStd::min(updatedHealth, GetMaxHealth());
        static_cast<NetworkHealthComponentController*>(GetController())->SetHealth(updatedHealth);
    }

    void NetworkHealthComponent::OnHealthChangedEvent([[maybe_unused]] const uint8_t& health)
    {
        // Hook for gameplay events such as player death, player revive, showing damage numbers, etc.
        ;
    }

    NetworkHealthComponentController::NetworkHealthComponentController(NetworkHealthComponent& parent)
        : NetworkHealthComponentControllerBase(parent)
    {
    }

    void NetworkHealthComponentController::OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        ;
    }

    void NetworkHealthComponentController::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        ;
    }
}
