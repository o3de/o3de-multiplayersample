/*
 * Copyright (c) Contributors to the Open 3D Engine Project
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <RigidBodyComponent.h>
#include <Components/PerfTest/NetworkTestComponent.h>

namespace MultiplayerSample
{
    NetworkTestComponentController::NetworkTestComponentController(NetworkTestComponent& parent)
        : NetworkTestComponentControllerBase(parent)
    {
    }

    void NetworkTestComponentController::OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        if (GetParent().GetEnableHopping())
        {
            m_accumulatedTime = 0.f;
            AZ::TickBus::Handler::BusConnect();
        }
    }

    void NetworkTestComponentController::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        AZ::TickBus::Handler::BusDisconnect();
    }

    void NetworkTestComponentController::OnTick(float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time)
    {
        m_accumulatedTime += deltaTime;

        if (m_accumulatedTime > GetParent().GetHopPeriod())
        {
            m_accumulatedTime = 0.f;

            if (PhysX::RigidBodyComponent* body = GetEntity()->FindComponent<PhysX::RigidBodyComponent>())
            {
                body->ApplyLinearImpulse(AZ::Vector3::CreateAxisZ(GetParent().GetHopForce()));
            }
        }
    }
}
