/*
 * Copyright (c) Contributors to the Open 3D Engine Project
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <Source/AutoGen/NetworkRandomImpulseComponent.AutoComponent.h>

namespace MultiplayerSample
{
    class NetworkRandomImpulseComponentController
        : public NetworkRandomImpulseComponentControllerBase
    {
    public:
        NetworkRandomImpulseComponentController(NetworkRandomImpulseComponent& parent);

        void OnActivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
        void OnDeactivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
        
    private:
        float m_accumulatedTime = 0.f;

        AZ::ScheduledEvent m_tickEvent;
        void TickEvent();
    };
}
