/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <Source/Components/NetworkHealthComponent.h>

namespace MultiplayerSample
{
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

#if AZ_TRAIT_SERVER_ENABLED
    void NetworkHealthComponentController::HandleSendHealthDelta([[maybe_unused]] AzNetworking::IConnection* invokingConnection, const float& healthDelta)
    {
        float health = GetHealth();
        health = AZStd::max(0.0f, AZStd::min(GetMaxHealth(), health + healthDelta));
        SetHealth(health);
    }
#endif
}
