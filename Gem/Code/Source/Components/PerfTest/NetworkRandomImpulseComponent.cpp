/*
 * Copyright (c) Contributors to the Open 3D Engine Project
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <RigidBodyComponent.h>
#include <Components/PerfTest/NetworkRandomImpulseComponent.h>

namespace MultiplayerSample
{
    NetworkRandomImpulseComponentController::NetworkRandomImpulseComponentController(NetworkRandomImpulseComponent& parent)
        : NetworkRandomImpulseComponentControllerBase(parent)
        , m_tickEvent{ [this] { TickEvent(); }, AZ::Name{ "NetworkRandomImpulseComponent" } }
    {
    }

    void NetworkRandomImpulseComponentController::OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        if (GetParent().GetEnableHopping())
        {
            m_tickEvent.Enqueue(AZ::TimeMs{ 0 }, true);
            m_accumulatedTime = 0.f;
        }
    }

    void NetworkRandomImpulseComponentController::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
    }

    void NetworkRandomImpulseComponentController::TickEvent()
    {
        const float deltaTime = static_cast<float>(m_tickEvent.TimeInQueueMs()) / 1000.f;
        m_accumulatedTime += deltaTime;

        if (m_accumulatedTime > GetParent().GetHopPeriod())
        {
            m_accumulatedTime = 0.f;

            if (PhysX::RigidBodyComponent* body = GetEntity()->FindComponent<PhysX::RigidBodyComponent>())
            {
                const AZ::Quaternion rotation = GetEntity()->GetTransform()->GetWorldRotationQuaternion();
                body->ApplyLinearImpulse(rotation.TransformVector(AZ::Vector3::CreateAxisZ(GetParent().GetHopForce())));
            }
        }
    }
}
