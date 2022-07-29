/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/Component/TickBus.h>
#include <Multiplayer/Components/NetBindComponent.h>

#include <Source/AutoGen/NetworkMatchComponent.AutoComponent.h>

namespace MultiplayerSample
{
    class NetworkMatchComponentController
        : public NetworkMatchComponentControllerBase
        , private AZ::TickBus::Handler
    {
    public:
        NetworkMatchComponentController(NetworkMatchComponent& parent);

        void OnActivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
        void OnDeactivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;

        void EndMatch();
        void EndRound();
    private:
        //! AZ::TickBus interface
        //! @{
        void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;
        int GetTickOrder() override;
        //! @}
    };
}
