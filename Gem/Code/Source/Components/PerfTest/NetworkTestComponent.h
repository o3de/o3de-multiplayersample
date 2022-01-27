/*
 * Copyright (c) Contributors to the Open 3D Engine Project
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/Component/TickBus.h>
#include <Source/AutoGen/NetworkTestComponent.AutoComponent.h>

namespace MultiplayerSample
{
    class NetworkTestComponentController
        : public NetworkTestComponentControllerBase
        , public AZ::TickBus::Handler
    {
    public:
        NetworkTestComponentController(NetworkTestComponent& parent);

        void OnActivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
        void OnDeactivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;

        //! AZ::TickBus overrides.
        //! @{
        void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;
        //! }@

    private:
        float m_accumulatedTime = 0.f;
    };
}
