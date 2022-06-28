/*
 * Copyright (c) Contributors to the Open 3D Engine Project
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <Source/AutoGen/NetworkRandomTranslateComponent.AutoComponent.h>
#include <AzCore/Component/TickBus.h>
#include <AzCore/Math/Vector3.h>
#include <AzCore/Math/Random.h>

namespace MultiplayerSample
{
    class NetworkRandomTranslateComponentController
        : public NetworkRandomTranslateComponentControllerBase,
          AZ::TickBus::Handler
    {
    public:
        NetworkRandomTranslateComponentController(NetworkRandomTranslateComponent& parent);

        //////////////////////////////////////////////////////////////////////////
        // NetworkRandomTranslateComponentControllerBase overrides
        void OnActivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
        void OnDeactivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
        //////////////////////////////////////////////////////////////////////////

    private:
        //////////////////////////////////////////////////////////////////////////
        // AZ::TickBus::Handler overrides
        void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;
        //////////////////////////////////////////////////////////////////////////

        AZ::Vector3 CalculateNextDestination();

        AZ::Vector3 m_originalPosition = AZ::Vector3::CreateZero();
        AZ::Vector3 m_destination = AZ::Vector3::CreateZero();
        float m_travelTime = 0.0f;
        AZ::SimpleLcgRandom m_simpleLcgRandom;
    };
}
