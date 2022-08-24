/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <Source/AutoGen/PlayerKnockbackEffectComponent.AutoComponent.h>

namespace MultiplayerSample
{
    class PlayerKnockbackEffectComponentController
        : public PlayerKnockbackEffectComponentControllerBase
    {
    public:
        explicit PlayerKnockbackEffectComponentController(PlayerKnockbackEffectComponent& parent);

        void OnActivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
        void OnDeactivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;

        void HandleRPC_Knockback(AzNetworking::IConnection* invokingConnection, const AZ::Vector3& direction) override;
    };
}
