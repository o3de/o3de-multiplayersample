/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of
 * this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <Source/Components/NetworkPlayerSpawnerComponent.h>
#include <Source/Spawners/IPlayerSpawner.h>

namespace MultiplayerSample
{
    void NetworkPlayerSpawnerComponent::OnInit()
    {
        ;
    }

    void NetworkPlayerSpawnerComponent::OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        AZ::Interface<MultiplayerSample::IPlayerSpawner>::Get()->RegisterPlayerSpawner(this);
    }

    void NetworkPlayerSpawnerComponent::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        AZ::Interface<MultiplayerSample::IPlayerSpawner>::Get()->UnregisterPlayerSpawner(this);
    }
} // namespace MultiplayerSample
