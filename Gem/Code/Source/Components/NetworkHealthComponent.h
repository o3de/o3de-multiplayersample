/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <Source/AutoGen/NetworkHealthComponent.AutoComponent.h>

namespace MultiplayerSample
{
    class NetworkHealthComponent
        : public NetworkHealthComponentBase
    {
    public:
        AZ_MULTIPLAYER_COMPONENT(MultiplayerSample::NetworkHealthComponent, s_networkHealthComponentConcreteUuid, MultiplayerSample::NetworkHealthComponentBase);

        static void Reflect(AZ::ReflectContext* context);

        NetworkHealthComponent();

        void OnInit() override;
        void OnActivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
        void OnDeactivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;

        //! Component accessor to set health, accounting for a configurable max and floor of 0
        //! @param updatedHealth the new health value
        void SetHealth(float updatedHealth);

    private:
        void OnHealthChangedEvent(const float& health);

        AZ::Event<float>::Handler m_healthEventHandler;
    };

    class NetworkHealthComponentController
        : public NetworkHealthComponentControllerBase
    {
    public:
        NetworkHealthComponentController(NetworkHealthComponent& parent);

        void OnActivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
        void OnDeactivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
    };
}
