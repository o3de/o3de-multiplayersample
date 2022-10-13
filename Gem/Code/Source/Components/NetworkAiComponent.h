/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 * 
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <Source/AutoGen/NetworkAiComponent.AutoComponent.h>

#include <AzCore/Math/Random.h>

namespace MultiplayerSample
{
    class NetworkWeaponsComponentController;
    class NetworkPlayerMovementComponentController;

    //! The NetworkAiComponent, when active, can execute behaviors and produce synthetic inputs to drive the
    //! NetworkPlayerMovementComponentController and NetworkWeaponsComponentController.
    class NetworkAiComponentController
        : public NetworkAiComponentControllerBase
    {
    public:
        NetworkAiComponentController(NetworkAiComponent& parent);

        void OnActivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
        void OnDeactivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;

        void TickMovement(NetworkPlayerMovementComponentController& movementController, float deltaTime);
        void TickWeapons(NetworkWeaponsComponentController& weaponsController, float deltaTime);

    private:
        friend class NetworkStressTestComponentController;
        void ConfigureAi(
            float fireIntervalMinMs, float fireIntervalMaxMs, float actionIntervalMinMs, float actionIntervalMaxMs, uint64_t seed);

        // TODO: Technically this guy should also be authority to autonomous so we don't roll different values after a migration..
        AZ::SimpleLcgRandom m_lcg;
    };
}
