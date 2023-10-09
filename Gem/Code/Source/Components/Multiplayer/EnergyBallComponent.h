/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/Component/EntityBus.h>
#include <Source/AutoGen/EnergyBallComponent.AutoComponent.h>
#include <Source/Weapons/WeaponGathers.h>

namespace MultiplayerSample
{
    class EnergyBallComponent
        : public EnergyBallComponentBase
        , public AZ::EntityBus::Handler
    {
    public:
        AZ_MULTIPLAYER_COMPONENT(MultiplayerSample::EnergyBallComponent, s_energyBallComponentConcreteUuid, MultiplayerSample::EnergyBallComponentBase);

        static void Reflect(AZ::ReflectContext* context);

        void OnActivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
        void OnDeactivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;

    private:
#if AZ_TRAIT_CLIENT
        void OnEntityDeactivated(const AZ::EntityId&) override;
        void DebugDraw();

        AZ::ScheduledEvent m_debugDrawEvent{ [this]()
        {
            DebugDraw();
        }, AZ::Name("EnergyBallDebugDraw") };
#endif

        GameEffect m_effect;
    };

    class EnergyBallComponentController
        : public EnergyBallComponentControllerBase
    {
    public:
        explicit EnergyBallComponentController(EnergyBallComponent& parent);

        void OnActivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
        void OnDeactivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;

#if AZ_TRAIT_SERVER
        void HandleRPC_LaunchBall(AzNetworking::IConnection* invokingConnection, const AZ::Vector3& startingPosition, const AZ::Vector3& direction, const Multiplayer::NetEntityId& owningNetEntityId) override;
        void CheckForCollisions();
        void KillEnergyBall();

    private:
        AZ::ScheduledEvent m_collisionCheckEvent{ [this]()
        {
            CheckForCollisions();
        }, AZ::Name("EnergyBallCheckForCollisions") };

        AZ::ScheduledEvent m_killEvent{ [this]()
        {
            KillEnergyBall();
        }, AZ::Name("KillEnergyBall") };

        AZ::Vector3 m_direction = AZ::Vector3::CreateZero();
        AZ::Transform m_lastSweepTransform = AZ::Transform::CreateIdentity();
        Multiplayer::NetEntityId m_shooterNetEntityId = Multiplayer::InvalidNetEntityId;
        NetEntityIdSet m_filteredNetEntityIds;
#endif
    };
}
