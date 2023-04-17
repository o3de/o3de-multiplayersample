/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/Component/TickBus.h>
#include <Source/AutoGen/EnergyBallComponent.AutoComponent.h>
#include <Source/Weapons/WeaponGathers.h>

namespace MultiplayerSample
{
    class EnergyBallComponent
        : public EnergyBallComponentBase
        , public AZ::TickBus::Handler
    {
    public:
        AZ_MULTIPLAYER_COMPONENT(MultiplayerSample::EnergyBallComponent, s_energyBallComponentConcreteUuid, MultiplayerSample::EnergyBallComponentBase);

        static void Reflect(AZ::ReflectContext* context);

        void OnActivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
        void OnDeactivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;

        void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;
#if AZ_TRAIT_CLIENT
        void HandleRPC_BallExplosion(AzNetworking::IConnection* invokingConnection, const HitEvent& hitEvent) override;
#endif

    private:
#if AZ_TRAIT_CLIENT
        void DebugDraw();
#endif
        // Track the previous "ball active" state so we know whether or not to flip the energy ball particle effect state.
        bool m_wasActive = true;
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
        void HandleRPC_KillBall(AzNetworking::IConnection* invokingConnection) override;
        void CheckForCollisions();
        void HideEnergyBall();

    private:
        AZ::ScheduledEvent m_collisionCheckEvent{ [this]()
        {
            CheckForCollisions();
        }, AZ::Name("EnergyBallCheckForCollisions") };

        AZ::Vector3 m_direction = AZ::Vector3::CreateZero();
        AZ::Transform m_lastSweepTransform = AZ::Transform::CreateIdentity();
        Multiplayer::NetEntityId m_shooterNetEntityId = Multiplayer::InvalidNetEntityId;
        NetEntityIdSet m_filteredNetEntityIds;

        HitEvent m_hitEvent;
#endif
    };
}
