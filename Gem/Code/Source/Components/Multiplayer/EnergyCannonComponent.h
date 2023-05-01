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
    class EnergyCannonComponent
        : public EnergyCannonComponentBase
    {
    public:
        AZ_MULTIPLAYER_COMPONENT(MultiplayerSample::EnergyCannonComponent, s_energyCannonComponentConcreteUuid, MultiplayerSample::EnergyCannonComponentBase);

        static void Reflect(AZ::ReflectContext* context);

        void OnActivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
        void OnDeactivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;

#if AZ_TRAIT_CLIENT
        void HandleRPC_TriggerBuildup(AzNetworking::IConnection* invokingConnection) override;
#endif

    private:
        GameEffect m_effect;
    };

    class EnergyCannonComponentController
        : public EnergyCannonComponentControllerBase
    {
    public:
        explicit EnergyCannonComponentController(EnergyCannonComponent& parent);

        void OnActivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
        void OnDeactivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;

#if AZ_TRAIT_SERVER
    private:
        void OnTriggerBuildup();
        AZ::ScheduledEvent m_triggerBuildupEvent{ [this]()
        {
            OnTriggerBuildup();
        }, AZ::Name("BuildupEnergyCannon") };

        void OnFireEnergyBall();
        AZ::ScheduledEvent m_firingEvent{[this]()
        {
            OnFireEnergyBall();
        }, AZ::Name("FireEnergyCannon")};

        void OnKillEnergyBall();
        AZ::ScheduledEvent m_killEvent{ [this]()
        {
            OnKillEnergyBall();
        }, AZ::Name("KillEnergyBall") };
#endif
    };
}
