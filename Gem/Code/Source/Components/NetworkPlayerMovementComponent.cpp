/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 * 
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <Source/Components/NetworkPlayerMovementComponent.h>

#include <Source/Components/NetworkAiComponent.h>
#include <Multiplayer/Components/NetworkCharacterComponent.h>
#include <Source/Components/NetworkAnimationComponent.h>
#include <Source/Components/NetworkSimplePlayerCameraComponent.h>
#include <Multiplayer/Components/NetworkTransformComponent.h>
#include <AzCore/Time/ITime.h>
#include <AzFramework/Components/CameraBus.h>

namespace MultiplayerSample
{
    AZ_CVAR(float, cl_WasdStickAccel, 5.0f, nullptr, AZ::ConsoleFunctorFlags::Null, "The linear acceleration to apply to WASD inputs to simulate analog stick controls");
    AZ_CVAR(float, cl_AimStickScaleZ, 0.1f, nullptr, AZ::ConsoleFunctorFlags::Null, "The scaling to apply to aim and view adjustments");
    AZ_CVAR(float, cl_AimStickScaleX, 0.05f, nullptr, AZ::ConsoleFunctorFlags::Null, "The scaling to apply to aim and view adjustments");

    NetworkPlayerMovementComponentController::NetworkPlayerMovementComponentController(NetworkPlayerMovementComponent& parent)
        : NetworkPlayerMovementComponentControllerBase(parent)
        , m_updateAI{ [this] { UpdateAI(); }, AZ::Name{ "MovementControllerAi" } }
    {
        ;
    }

    void NetworkPlayerMovementComponentController::OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        NetworkAiComponent* networkAiComponent = GetParent().GetNetworkAiComponent();
        m_aiEnabled = (networkAiComponent != nullptr) ? networkAiComponent->GetEnabled() : false;
        if (m_aiEnabled)
        {
            m_updateAI.Enqueue(AZ::TimeMs{ 0 }, true);
            m_networkAiComponentController = GetNetworkAiComponentController();
        }
        else if (IsNetEntityRoleAutonomous())
        {
            StartingPointInput::InputEventNotificationBus::MultiHandler::BusConnect(MoveFwdEventId);
            StartingPointInput::InputEventNotificationBus::MultiHandler::BusConnect(MoveBackEventId);
            StartingPointInput::InputEventNotificationBus::MultiHandler::BusConnect(MoveLeftEventId);
            StartingPointInput::InputEventNotificationBus::MultiHandler::BusConnect(MoveRightEventId);
            StartingPointInput::InputEventNotificationBus::MultiHandler::BusConnect(SprintEventId);
            StartingPointInput::InputEventNotificationBus::MultiHandler::BusConnect(JumpEventId);
            StartingPointInput::InputEventNotificationBus::MultiHandler::BusConnect(CrouchEventId);
            StartingPointInput::InputEventNotificationBus::MultiHandler::BusConnect(LookLeftRightEventId);
            StartingPointInput::InputEventNotificationBus::MultiHandler::BusConnect(LookUpDownEventId);
            StartingPointInput::InputEventNotificationBus::MultiHandler::BusConnect(ZoomInEventId);
            StartingPointInput::InputEventNotificationBus::MultiHandler::BusConnect(ZoomOutEventId);
        }
    }

    void NetworkPlayerMovementComponentController::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        if (IsNetEntityRoleAutonomous() && !m_aiEnabled)
        {
            StartingPointInput::InputEventNotificationBus::MultiHandler::BusDisconnect(MoveFwdEventId);
            StartingPointInput::InputEventNotificationBus::MultiHandler::BusDisconnect(MoveBackEventId);
            StartingPointInput::InputEventNotificationBus::MultiHandler::BusDisconnect(MoveLeftEventId);
            StartingPointInput::InputEventNotificationBus::MultiHandler::BusDisconnect(MoveRightEventId);
            StartingPointInput::InputEventNotificationBus::MultiHandler::BusDisconnect(SprintEventId);
            StartingPointInput::InputEventNotificationBus::MultiHandler::BusDisconnect(JumpEventId);
            StartingPointInput::InputEventNotificationBus::MultiHandler::BusDisconnect(CrouchEventId);
            StartingPointInput::InputEventNotificationBus::MultiHandler::BusDisconnect(LookLeftRightEventId);
            StartingPointInput::InputEventNotificationBus::MultiHandler::BusDisconnect(LookUpDownEventId);
            StartingPointInput::InputEventNotificationBus::MultiHandler::BusDisconnect(ZoomInEventId);
            StartingPointInput::InputEventNotificationBus::MultiHandler::BusDisconnect(ZoomOutEventId);
        }
    }

    void NetworkPlayerMovementComponentController::CreateInput(Multiplayer::NetworkInput& input, float deltaTime)
    {
        // Movement axis
        // Since we're on a keyboard, this adds a touch of an acceleration curve to the keyboard inputs
        // This is so that tapping the keyboard moves the virtual stick less than just holding it down
        m_forwardWeight = std::min<float>(m_forwardDown ? m_forwardWeight + cl_WasdStickAccel * deltaTime : 0.0f, 1.0f);
        m_leftWeight = std::min<float>(m_leftDown ? m_leftWeight + cl_WasdStickAccel * deltaTime : 0.0f, 1.0f);
        m_backwardWeight = std::min<float>(m_backwardDown ? m_backwardWeight + cl_WasdStickAccel * deltaTime : 0.0f, 1.0f);
        m_rightWeight = std::min<float>(m_rightDown ? m_rightWeight + cl_WasdStickAccel * deltaTime : 0.0f, 1.0f);

        // Inputs for your own component always exist
        NetworkPlayerMovementComponentNetworkInput* playerInput = input.FindComponentInput<NetworkPlayerMovementComponentNetworkInput>();

        playerInput->m_forwardAxis = StickAxis(m_forwardWeight - m_backwardWeight);
        playerInput->m_strafeAxis = StickAxis(m_leftWeight - m_rightWeight);

        // View Axis
        playerInput->m_viewYaw = MouseAxis(m_viewYaw);
        playerInput->m_viewPitch = MouseAxis(m_viewPitch);

        // Strafe input
        playerInput->m_sprint = m_sprinting;
        playerInput->m_jump = m_jumping;
        playerInput->m_crouch = m_crouching;

        // Just a note for anyone who is super confused by this, ResetCount is a predictable network property, it gets set on the client
        // through correction packets
        playerInput->m_resetCount = GetNetworkTransformComponentController()->GetResetCount();
    }

    void NetworkPlayerMovementComponentController::ProcessInput(Multiplayer::NetworkInput& input, float deltaTime)
    {
        // If the input reset count doesn't match the state's reset count it can mean two things:
        //  1) On the server: we were reset and we are now receiving inputs from the client for an old reset count
        //  2) On the client: we were reset and we are replaying old inputs after being corrected
        // In both cases we don't want to process these inputs
        NetworkPlayerMovementComponentNetworkInput* playerInput = input.FindComponentInput<NetworkPlayerMovementComponentNetworkInput>();
        if (playerInput->m_resetCount != GetNetworkTransformComponentController()->GetResetCount())
        {
            return;
        }

        GetNetworkAnimationComponentController()->ModifyActiveAnimStates().SetBit(
            aznumeric_cast<uint32_t>(CharacterAnimState::Sprinting), playerInput->m_sprint);
        GetNetworkAnimationComponentController()->ModifyActiveAnimStates().SetBit(
            aznumeric_cast<uint32_t>(CharacterAnimState::Jumping), playerInput->m_jump);
        GetNetworkAnimationComponentController()->ModifyActiveAnimStates().SetBit(
            aznumeric_cast<uint32_t>(CharacterAnimState::Crouching), playerInput->m_crouch);

        // Update orientation
        AZ::Vector3 aimAngles = GetNetworkSimplePlayerCameraComponentController()->GetAimAngles();
        aimAngles.SetZ(NormalizeHeading(aimAngles.GetZ() - playerInput->m_viewYaw * cl_AimStickScaleZ));
        aimAngles.SetX(NormalizeHeading(aimAngles.GetX() - playerInput->m_viewPitch * cl_AimStickScaleX));
        aimAngles.SetX(
            NormalizeHeading(AZ::GetClamp(aimAngles.GetX(), -AZ::Constants::QuarterPi * 0.75f, AZ::Constants::QuarterPi * 0.75f)));
        GetNetworkSimplePlayerCameraComponentController()->SetAimAngles(aimAngles);

        const AZ::Quaternion newOrientation = AZ::Quaternion::CreateRotationZ(aimAngles.GetZ());
        GetEntity()->GetTransform()->SetLocalRotationQuaternion(newOrientation);

        // Update velocity
        UpdateVelocity(*playerInput);

        GetNetworkCharacterComponentController()->TryMoveWithVelocity(GetVelocity(), deltaTime);
    }

    void NetworkPlayerMovementComponentController::UpdateVelocity(const NetworkPlayerMovementComponentNetworkInput& playerInput)
    {
        const float fwdBack = playerInput.m_forwardAxis;
        const float leftRight = playerInput.m_strafeAxis;

        float speed = 0.0f;
        if (playerInput.m_crouch)
        {
            speed = GetCrouchSpeed();
        }
        else if (fwdBack < 0.0f)
        {
            speed = GetReverseSpeed();
        }
        else
        {
            if (playerInput.m_sprint)
            {
                speed = GetSprintSpeed();
            }
            else
            {
                speed = GetWalkSpeed();
            }
        }

        // Not moving?
        if (fwdBack == 0.0f && leftRight == 0.0f)
        {
            SetVelocity(AZ::Vector3::CreateZero());
        }
        else
        {
            const float stickInputAngle = AZ::Atan2(leftRight, fwdBack);
            const float currentHeading = GetNetworkTransformComponentController()->GetRotation().GetEulerRadians().GetZ();
            const float targetHeading =
                NormalizeHeading(currentHeading + stickInputAngle); // Update current heading with stick input angles
            const AZ::Vector3 fwd = AZ::Vector3::CreateAxisY();
            SetVelocity(AZ::Quaternion::CreateRotationZ(targetHeading).TransformVector(fwd) * speed);
        }
    }

    float NetworkPlayerMovementComponentController::NormalizeHeading(float heading) const
    {
        // Ensure heading in range [-pi, +pi]
        if (heading > AZ::Constants::Pi)
        {
            return static_cast<float>(heading - AZ::Constants::TwoPi);
        }
        else if (heading < -AZ::Constants::Pi)
        {
            return static_cast<float>(heading + AZ::Constants::TwoPi);
        }
        return heading;
    }

    void NetworkPlayerMovementComponentController::OnPressed(float value)
    {
        const StartingPointInput::InputEventNotificationId* inputId = StartingPointInput::InputEventNotificationBus::GetCurrentBusId();

        if (inputId == nullptr)
        {
            return;
        }
        else if (*inputId == MoveFwdEventId)
        {
            m_forwardDown = true;
        }
        else if (*inputId == MoveBackEventId)
        {
            m_backwardDown = true;
        }
        else if (*inputId == MoveLeftEventId)
        {
            m_leftDown = true;
        }
        else if (*inputId == MoveRightEventId)
        {
            m_rightDown = true;
        }
        else if (*inputId == SprintEventId)
        {
            m_sprinting = true;
        }
        else if (*inputId == JumpEventId)
        {
            m_jumping = true;
        }
        else if (*inputId == CrouchEventId)
        {
            m_crouching = true;
        }
        else if (*inputId == LookLeftRightEventId)
        {
            m_viewYaw = value;
        }
        else if (*inputId == LookUpDownEventId)
        {
            m_viewPitch = value;
        }
    }

    void NetworkPlayerMovementComponentController::OnReleased(float value)
    {
        const StartingPointInput::InputEventNotificationId* inputId = StartingPointInput::InputEventNotificationBus::GetCurrentBusId();

        if (inputId == nullptr)
        {
            return;
        }
        else if (*inputId == MoveFwdEventId)
        {
            m_forwardDown = false;
        }
        else if (*inputId == MoveBackEventId)
        {
            m_backwardDown = false;
        }
        else if (*inputId == MoveLeftEventId)
        {
            m_leftDown = false;
        }
        else if (*inputId == MoveRightEventId)
        {
            m_rightDown = false;
        }
        else if (*inputId == SprintEventId)
        {
            m_sprinting = false;
        }
        else if (*inputId == JumpEventId)
        {
            m_jumping = false;
        }
        else if (*inputId == CrouchEventId)
        {
            m_crouching = false;
        }
        else if (*inputId == LookLeftRightEventId)
        {
            m_viewYaw = value;
        }
        else if (*inputId == LookUpDownEventId)
        {
            m_viewPitch = value;
        }
    }

    void NetworkPlayerMovementComponentController::OnHeld(float value)
    {
        const StartingPointInput::InputEventNotificationId* inputId = StartingPointInput::InputEventNotificationBus::GetCurrentBusId();

        if (inputId == nullptr)
        {
            return;
        }
        else if (*inputId == LookLeftRightEventId)
        {
            m_viewYaw = value;
        }
        else if (*inputId == LookUpDownEventId)
        {
            m_viewPitch = value;
        }
    }

    void NetworkPlayerMovementComponentController::UpdateAI()
    {
        float deltaTime = static_cast<float>(m_updateAI.TimeInQueueMs()) / 1000.f;
        if (m_networkAiComponentController != nullptr)
        {
            m_networkAiComponentController->TickMovement(*this, deltaTime);
        }
    }
} // namespace MultiplayerSample
