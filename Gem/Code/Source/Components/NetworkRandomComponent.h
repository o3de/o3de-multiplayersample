/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <Source/AutoGen/NetworkRandomComponent.AutoComponent.h>

#include <AzCore/Math/Random.h>

namespace MultiplayerSample
{
    class NetworkRandomComponent
        : public NetworkRandomComponentBase
    {
    public:
        AZ_MULTIPLAYER_COMPONENT(MultiplayerSample::NetworkRandomComponent, s_networkRandomComponentConcreteUuid, MultiplayerSample::NetworkRandomComponentBase);

        static void Reflect(AZ::ReflectContext* context);

        NetworkRandomComponent();

        void OnInit() override;
        void OnActivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
        void OnDeactivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;

        void RollRandom();

    private:
        void OnSeedChangedEvent(const uint64_t& seed);

        AZ::Event<uint64_t>::Handler m_seedEventHandler;
        AZ::SimpleLcgRandom m_simpleRng;
        bool m_seedInitialized = false;
    };

    class NetworkRandomComponentController
        : public NetworkRandomComponentControllerBase
    {
    public:
        NetworkRandomComponentController(NetworkRandomComponent& parent);

        void OnActivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
        void OnDeactivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
    };
}
