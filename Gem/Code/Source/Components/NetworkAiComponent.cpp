/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <Source/Components/NetworkAiComponent.h>

#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <Source/Components/WasdPlayerMovementComponent.h>
#include <Source/Components/NetworkWeaponsComponent.h>
#include <AzCore/Time/ITime.h>
#include <Multiplayer/Components/NetBindComponent.h>

namespace MultiplayerSample
{
    constexpr static float SecondsToMs = 1000.f;

    void NetworkAiComponent::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<NetworkAiComponent, NetworkAiComponentBase>()->Version(1);
        }

        NetworkAiComponentBase::Reflect(context);
    }

    void NetworkAiComponent::OnInit()
    {
    }

    void NetworkAiComponent::OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
    }

    void NetworkAiComponent::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
    }

    void NetworkAiComponent::TickMovement(WasdPlayerMovementComponentController& movementController, float deltaTime)
    {
        // TODO: Execute this tick only if this component is owned by this endpoint (currently ticks on server only)
        float deltaTimeMs = deltaTime * SecondsToMs;
        m_remainingTimeMs -= deltaTimeMs;

        if (m_remainingTimeMs <= 0)
        {
            // Determine a new directive after 500 to 9500 ms
            m_remainingTimeMs = m_lcg.GetRandomFloat() * (m_actionIntervalMaxMs - m_actionIntervalMinMs) + m_actionIntervalMinMs;
            m_turnRate = 1.f / m_remainingTimeMs;

            // Randomize new target yaw and pitch and compute the delta from the current yaw and pitch respectively
            m_targetYawDelta = -movementController.m_viewYaw + (m_lcg.GetRandomFloat() * 2.f - 1.f);
            m_targetPitchDelta = -movementController.m_viewPitch + (m_lcg.GetRandomFloat() - 0.5f);

            // Randomize the action and strafe direction (used only if we decide to strafe)
            m_action = static_cast<Action>(m_lcg.GetRandom() % static_cast<int>(Action::COUNT));
            m_strafingRight = static_cast<bool>(m_lcg.GetRandom() % 2);
        }

        // Translate desired motion into inputs

        // Interpolate the current view yaw and pitch values towards the desired values
        movementController.m_viewYaw += m_turnRate * deltaTimeMs * m_targetYawDelta;
        movementController.m_viewPitch += m_turnRate * deltaTimeMs * m_targetPitchDelta;

        // Reset keyboard movement inputs decided on the previous frame
        movementController.m_forwardDown = false;
        movementController.m_backwardDown = false;
        movementController.m_leftDown = false;
        movementController.m_rightDown = false;
        movementController.m_sprinting = false;
        movementController.m_jumping = false;
        movementController.m_crouching = false;

        switch (m_action)
        {
        case Action::Default:
            movementController.m_forwardDown = true;
            break;
        case Action::Sprinting:
            movementController.m_forwardDown = true;
            movementController.m_sprinting = true;
            break;
        case Action::Jumping:
            movementController.m_forwardDown = true;
            movementController.m_jumping = true;
            break;
        case Action::Crouching:
            movementController.m_forwardDown = true;
            movementController.m_crouching = true;
            break;
        case Action::Strafing:
            if (m_strafingRight)
            {
                movementController.m_rightDown = true;
            }
            else
            {
                movementController.m_leftDown = true;
            }
            break;
        default:
            break;
        }
    }

    void NetworkAiComponent::TickWeapons(NetworkWeaponsComponentController& weaponsController, float deltaTime)
    {
        // TODO: Execute this tick only if this component is owned by this endpoint (currently ticks on server only)
        m_timeToNextShot -= deltaTime * SecondsToMs;
        if (m_timeToNextShot <= 0)
        {
            if (m_shotFired)
            {
                // Fire weapon between 100 and 10000 ms from now
                m_timeToNextShot = m_lcg.GetRandomFloat() * (m_fireIntervalMaxMs - m_fireIntervalMinMs) + m_fireIntervalMinMs;
                m_shotFired = false;
                weaponsController.m_weaponFiring = false;
            }
            else
            {
                weaponsController.m_weaponFiring = true;
                m_shotFired = true;
            }
        }
    }

    void NetworkAiComponent::ConfigureAi(
            float fireIntervalMinMs, float fireIntervalMaxMs, float actionIntervalMinMs, float actionIntervalMaxMs, uint64_t seed)
    {
        m_fireIntervalMinMs = fireIntervalMinMs;
        m_fireIntervalMaxMs = fireIntervalMaxMs;
        m_actionIntervalMinMs = actionIntervalMinMs;
        m_actionIntervalMaxMs = actionIntervalMaxMs;
        m_lcg.SetSeed(seed);
    }
}
