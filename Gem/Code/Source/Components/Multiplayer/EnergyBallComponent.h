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

        void HandleRPC_LaunchBall(AzNetworking::IConnection* invokingConnection, const AZ::Vector3& startingPosition, const AZ::Vector3& direction) override;

    private:
        void OnCollisionBegin(const AzPhysics::CollisionEvent& collisionEvent);
        AzPhysics::SimulatedBodyEvents::OnCollisionBegin::Handler m_collisionHandler{ [this](
            AzPhysics::SimulatedBodyHandle, const AzPhysics::CollisionEvent& collisionEvent)
        {
            OnCollisionBegin(collisionEvent);
        } };
    };
}
