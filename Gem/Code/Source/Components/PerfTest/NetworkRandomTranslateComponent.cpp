/*
 * Copyright (c) Contributors to the Open 3D Engine Project
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <Components/PerfTest/NetworkRandomTranslateComponent.h>
#include <AzCore/Component/TransformBus.h>
#include <AzCore/std/time.h>

namespace MultiplayerSample
{
    NetworkRandomTranslateComponentController::NetworkRandomTranslateComponentController(NetworkRandomTranslateComponent& parent)
        : NetworkRandomTranslateComponentControllerBase(parent), m_simpleLcgRandom(AZStd::GetTimeUTCMilliSecond())
    {
    }

    void NetworkRandomTranslateComponentController::OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        m_originalPosition = GetParent().GetEntity()->GetTransform()->GetWorldTranslation();
        m_destination = CalculateNextDestination();
        AZ::TickBus::Handler::BusConnect();
    }

    void NetworkRandomTranslateComponentController::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        AZ::TickBus::Handler::BusDisconnect();
    }

    void NetworkRandomTranslateComponentController::OnTick(float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time)
    {
        m_travelTime += deltaTime;

        const AZ::Vector3 currentPosition = GetParent().GetEntity()->GetTransform()->GetWorldTranslation();
        const float t = m_travelTime/GetParent().GetMovementDuration();
        const AZ::Vector3 newPosition = currentPosition.Lerp(m_destination, t);
        GetParent().GetEntity()->GetTransform()->SetWorldTranslation(newPosition);
        
        if (m_travelTime > GetParent().GetMovementDuration())
        {
            m_travelTime = 0.0f;
            m_destination = CalculateNextDestination();
        }
    }

    AZ::Vector3 NetworkRandomTranslateComponentController::CalculateNextDestination()
    {
        AZ::Vector3 random(0.5f - m_simpleLcgRandom.GetRandomFloat(), 0.5f - m_simpleLcgRandom.GetRandomFloat(), 0.5f - m_simpleLcgRandom.GetRandomFloat());
        random = GetParent().GetMaxMoveDistance() * random.GetNormalizedEstimate();
        return m_originalPosition+random;
    }
}
