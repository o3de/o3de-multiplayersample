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
        // TODO: Provide an interface for controlling the seed
        m_lcg.SetSeed(static_cast<uint64_t>(AZ::Interface<AZ::ITime>::Get()->GetElapsedTimeMs()));
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
        m_remainingTimeMs -= deltaTime * 1000;

        if (m_remainingTimeMs <= 0)
        {
            // Determine a new directive after 500 to 9500 ms
            m_remainingTimeMs = m_lcg.GetRandomFloat() * 9500.f + 500.f;
            m_turnRate = 1.f / m_remainingTimeMs;
            m_targetYawDelta = -movementController.m_viewYaw + (m_lcg.GetRandomFloat() * 2.f - 1.f);
            m_targetPitchDelta = -movementController.m_viewPitch + (m_lcg.GetRandomFloat() - 0.5f);
            m_action = static_cast<Action>(m_lcg.GetRandom() % static_cast<int>(Action::COUNT));
            m_strafingRight = static_cast<bool>(m_lcg.GetRandom() % 2);
        }

        // Translate desired motion into inputs

        // Interpolate the current view yaw and pitch values towards the desired values
        movementController.m_viewYaw += 1000.f * m_turnRate * deltaTime * m_targetYawDelta;
        movementController.m_viewPitch += 1000.f * m_turnRate * deltaTime * m_targetPitchDelta;

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
        m_timeToNextShot -= deltaTime * 1000;
        if (m_timeToNextShot <= 0)
        {
            if (m_shotFired)
            {
                // Fire weapon between 100 and 10000 ms from now
                m_timeToNextShot = m_lcg.GetRandomFloat() * 9900.f + 100.f;
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
}
