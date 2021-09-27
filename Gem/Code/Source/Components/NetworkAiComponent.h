/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 * 
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <Source/AutoGen/NetworkAiComponent.AutoComponent.h>

#include <AzCore/Math/Random.h>

namespace MultiplayerSample
{
    class NetworkWeaponsComponentController;
    class NetworkPlayerMovementComponentController;


    // The NetworkAiComponent, when active, can execute behaviors and produce synthetic inputs to drive the
    // NetworkPlayerMovementComponentController and NetworkWeaponsComponentController.
    class NetworkAiComponent : public NetworkAiComponentBase
    {
    public:
        AZ_MULTIPLAYER_COMPONENT(NetworkAiComponent, s_networkAiComponentConcreteUuid, NetworkAiComponentBase);

        static void Reflect(AZ::ReflectContext* context);

        using NetworkAiComponentBase::NetworkAiComponentBase;

        // MultiplayerComponent interface
        void OnInit() override;
        void OnActivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
        void OnDeactivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;

        void TickMovement(NetworkPlayerMovementComponentController& movementController, float deltaTime);
        void TickWeapons(NetworkWeaponsComponentController& weaponsController, float deltaTime);

    private:
        friend class NetworkStressTestComponentController;
        void ConfigureAi(
            float fireIntervalMinMs, float fireIntervalMaxMs, float actionIntervalMinMs, float actionIntervalMaxMs, uint64_t seed);

        AZ::SimpleLcgRandom m_lcg;

        // Our "AI" is really just a chaos monkey. Every N ms, we choose a cardinal direction to move towards,
        // and flip coins to determine if we should shoot, or perform some other action.
        float m_remainingTimeMs = 0.f;
        float m_fireIntervalMinMs = 100.f;
        float m_fireIntervalMaxMs = 10000.f;
        float m_actionIntervalMinMs = 500.f;
        float m_actionIntervalMaxMs = 10000.f;

        float m_turnRate = 0.f;
        float m_targetYawDelta = 0.f;
        float m_targetPitchDelta = 0.f;

        enum class Action
        {
            Default,
            Strafing,
            Sprinting,
            Jumping,
            Crouching,
            COUNT = Crouching + 1
        };
        Action m_action = Action::Default;
        bool m_strafingRight = false;

        bool m_shotFired = true;
        float m_timeToNextShot = 0.f;
    };
}
