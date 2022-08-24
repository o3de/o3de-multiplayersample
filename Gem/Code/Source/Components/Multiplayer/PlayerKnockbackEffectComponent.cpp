/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <PlayerKnockbackBus.h>
#include <Multiplayer/Components/NetworkCharacterComponent.h>
#include <Multiplayer/Components/NetworkTransformComponent.h>
#include <Source/Components/Multiplayer/PlayerKnockbackEffectComponent.h>

namespace MultiplayerSample
{
    PlayerKnockbackEffectComponentController::PlayerKnockbackEffectComponentController(PlayerKnockbackEffectComponent& parent)
        : PlayerKnockbackEffectComponentControllerBase(parent)
    {
    }

    void PlayerKnockbackEffectComponentController::OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
    }

    void PlayerKnockbackEffectComponentController::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
    }

    void PlayerKnockbackEffectComponentController::HandleRPC_Knockback([[maybe_unused]] AzNetworking::IConnection* invokingConnection,
        const AZ::Vector3& direction)
    {
        const AZ::Vector3& position = GetNetworkTransformComponentController()->GetTranslation();
        PlayerKnockbackNotificationBus::Event(GetEntityId(), &PlayerKnockbackNotificationBus::Events::OnPlayerKnockback, position, direction);

        // Spread out the move to avoid issues in a physical simulation.
        const float largestPossibleStep = 0.25f;
        const int steps = aznumeric_cast<int>(direction.GetLength() / largestPossibleStep) + 1;
        if (steps > 0)
        {
            const float deltaTime = 1.f / aznumeric_cast<float>(steps);
            for (int i = 0; i < steps; ++i)
            {
                GetNetworkCharacterComponentController()->TryMoveWithVelocity(direction, deltaTime);
            }
        }
    }
}
