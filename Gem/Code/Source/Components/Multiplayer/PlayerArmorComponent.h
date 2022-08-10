/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <Source/AutoGen/PlayerArmorComponent.AutoComponent.h>

namespace MultiplayerSample
{
    class PlayerArmorComponentController
        : public PlayerArmorComponentControllerBase
    {
    public:
        explicit PlayerArmorComponentController(PlayerArmorComponent& parent);

        void OnActivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
        void OnDeactivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
        
    private:
        void OnAmountChanged(float armor);
        AZ::Event<float>::Handler m_changedHandler{ [this](float armor)
        {
            OnAmountChanged(armor);
        } };
    };
}
