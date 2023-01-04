/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzFramework/Physics/Common/PhysicsEvents.h>
#include <Source/AutoGen/EnergyBallComponent.AutoComponent.h>

namespace MultiplayerSample
{
    class EnergyBallComponentController
        : public EnergyBallComponentControllerBase
    {
    public:
        explicit EnergyBallComponentController(EnergyBallComponent& parent);

        void OnActivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
        void OnDeactivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;

#if AZ_TRAIT_SERVER
        void HandleRPC_LaunchBall(AzNetworking::IConnection* invokingConnection, const AZ::Vector3& startingPosition, const AZ::Vector3& direction) override;
#endif

    private:
        void HideEnergyBall();

#if AZ_TRAIT_SERVER
        void TryKnockbackPlayer(AZ::Entity* target);
#endif

        void OnCollisionBegin(const AzPhysics::CollisionEvent& collisionEvent);
        AzPhysics::SimulatedBodyEvents::OnCollisionBegin::Handler m_collisionHandler{ [this](
            AzPhysics::SimulatedBodyHandle, const AzPhysics::CollisionEvent& collisionEvent)
        {
            OnCollisionBegin(collisionEvent);
        } };

        void LoadEnergyBallSettings();
        AZ::Vector3 m_direction = AZ::Vector3::CreateZero();
        double m_knockbackDistance = 0.0;
        double m_speed = 0.0;
        AZ::s64 m_armorDamage = 0;
    };
}
