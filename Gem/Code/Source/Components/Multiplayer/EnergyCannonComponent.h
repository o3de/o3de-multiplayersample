/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <Source/AutoGen/EnergyCannonComponent.AutoComponent.h>

namespace MultiplayerSample
{
    class EnergyCannonComponentController
        : public EnergyCannonComponentControllerBase
    {
    public:
        explicit EnergyCannonComponentController(EnergyCannonComponent& parent);

        void OnActivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
        void OnDeactivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;

    private:
        void OnFireEnergyBall();
        AZ::ScheduledEvent m_firingEvent{[this]()
        {
            OnFireEnergyBall();
        }, AZ::Name("EnergyCannonComponentController")};
    };
}
