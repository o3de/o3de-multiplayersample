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
        : m_healthEventHandler([this](const float& health) { OnHealthChangedEvent(health); })
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

    void NetworkHealthComponent::SetHealth(float updatedHealth)
    {
        updatedHealth = AZStd::max(0.f, AZStd::min(updatedHealth, GetMaxHealth()));
        static_cast<NetworkHealthComponentController*>(GetController())->SetHealth(updatedHealth);
    }

    void NetworkHealthComponent::OnHealthChangedEvent([[maybe_unused]] const float& health)
    {
        // Hook for gameplay events such as player death, player revive, showing damage numbers, etc.
        ;
    }
}
