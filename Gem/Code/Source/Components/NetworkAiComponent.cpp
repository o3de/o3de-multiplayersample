/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <Source/Components/NetworkAiComponent.h>
#include <Source/Components/NetworkPlayerMovementComponent.h>
#include <Source/Components/NetworkWeaponsComponent.h>
#include <Multiplayer/Components/NetBindComponent.h>
#include <Multiplayer/Components/LocalPredictionPlayerInputComponent.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Time/ITime.h>

namespace MultiplayerSample
{
    AZ_CVAR(bool, mps_botMode, false, nullptr, AZ::ConsoleFunctorFlags::Null, "If true, enable bot (AI) mode for client.");

    constexpr static float SecondsToMs = 1000.f;

    NetworkAiComponentController::NetworkAiComponentController(NetworkAiComponent& parent)
        : NetworkAiComponentControllerBase(parent)
    {
    }

    void NetworkAiComponentController::OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        if (IsNetEntityRoleAutonomous() && mps_botMode)
        {
            SetEnabled(true);
        }

        if (GetEnabled())
        {
            Multiplayer::LocalPredictionPlayerInputComponentController* playerInputController = GetLocalPredictionPlayerInputComponentController();
            if (playerInputController != nullptr)
            {
                playerInputController->ForceEnableAutonomousUpdate();
            }
        }
    }

    void NetworkAiComponentController::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        if (GetEnabled())
        {
            Multiplayer::LocalPredictionPlayerInputComponentController* playerInputController = GetLocalPredictionPlayerInputComponentController();
            if (playerInputController != nullptr)
            {
                playerInputController->ForceDisableAutonomousUpdate();
            }
        }
    }

    void NetworkAiComponentController::TickMovement(NetworkPlayerMovementComponentController& movementController, float deltaTime)
    {
        // TODO: Execute this tick only if this component is owned by this endpoint (currently ticks on server only)
        float deltaTimeMs = deltaTime * SecondsToMs;
        ModifyRemainingTimeMs() -= deltaTimeMs;

        if (GetRemainingTimeMs() <= 0)
        {
            // Determine a new directive after 500 to 9500 ms
            SetRemainingTimeMs(m_lcg.GetRandomFloat() * (GetActionIntervalMaxMs() - GetActionIntervalMinMs()) + GetActionIntervalMinMs());
            SetTurnRate(1.f / GetRemainingTimeMs());

            // Randomize new target yaw and pitch and compute the delta from the current yaw and pitch respectively
            SetTargetYawDelta(-movementController.m_viewYaw + (m_lcg.GetRandomFloat() * 2.f - 1.f));
            SetTargetPitchDelta(-movementController.m_viewPitch + (m_lcg.GetRandomFloat() - 0.5f));

            // Randomize the action and strafe direction (used only if we decide to strafe)
            SetAction(static_cast<Action>(m_lcg.GetRandom() % static_cast<int>(Action::COUNT)));
            SetStrafingRight(static_cast<bool>(m_lcg.GetRandom() % 2));
        }

        // Translate desired motion into inputs

        // Interpolate the current view yaw and pitch values towards the desired values
        movementController.m_viewYaw += GetTurnRate() * deltaTimeMs * GetTargetYawDelta();
        movementController.m_viewPitch += GetTurnRate() * deltaTimeMs * GetTargetPitchDelta();

        // Reset keyboard movement inputs decided on the previous frame
        movementController.m_forwardDown = false;
        movementController.m_backwardDown = false;
        movementController.m_leftDown = false;
        movementController.m_rightDown = false;
        movementController.m_sprinting = false;
        movementController.m_jumping = false;
        movementController.m_crouching = false;

        switch (GetAction())
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
            if (GetStrafingRight())
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

    void NetworkAiComponentController::TickWeapons(NetworkWeaponsComponentController& weaponsController, float deltaTime)
    {
        // TODO: Execute this tick only if this component is owned by this endpoint (currently ticks on server only)
        ModifyTimeToNextShot() -= deltaTime * SecondsToMs;
        if (GetTimeToNextShot() <= 0)
        {
            if (GetShotFired())
            {
                // Fire weapon between 100 and 10000 ms from now
                SetTimeToNextShot(m_lcg.GetRandomFloat() * (GetFireIntervalMaxMs() - GetFireIntervalMinMs()) + GetFireIntervalMinMs());
                SetShotFired(false);
                weaponsController.m_weaponFiring = false;
            }
            else
            {
                weaponsController.m_weaponFiring = true;
                SetShotFired(true);
            }
        }
    }

    void NetworkAiComponentController::ConfigureAi(
        float fireIntervalMinMs, float fireIntervalMaxMs, float actionIntervalMinMs, float actionIntervalMaxMs, uint64_t seed)
    {
        SetFireIntervalMinMs(fireIntervalMinMs);
        SetFireIntervalMaxMs(fireIntervalMaxMs);
        SetActionIntervalMinMs(actionIntervalMinMs);
        SetActionIntervalMaxMs(actionIntervalMaxMs);
        m_lcg.SetSeed(seed);
    }
}
