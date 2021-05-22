/*
 * All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
 * its licensors.
 *
 * For complete copyright and license terms please see the LICENSE at the root of this
 * distribution (the "License"). All use of this software is governed by the License,
 * or, if provided, by the license below or the license accompanying this file. Do not
 * remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *
 */

#include <Source/Components/WasdPlayerMovementComponent.h>
#include <Source/Components/CharacterComponent.h>
#include <Source/Components/NetworkAnimationComponent.h>
#include <Multiplayer/Components/NetworkTransformComponent.h>
#include <AzFramework/Visibility/EntityBoundsUnionBus.h>

namespace MultiplayerSample
{
    AZ_CVAR(float, cl_WasdStickAccel, 5.0f, nullptr, AZ::ConsoleFunctorFlags::Null, "The linear acceleration to apply to WASD inputs to simulate analog stick controls");

    void WasdPlayerMovementComponent::WasdPlayerMovementComponent::Reflect(AZ::ReflectContext* context)
    {
        AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context);
        if (serializeContext)
        {
            serializeContext->Class<WasdPlayerMovementComponent, WasdPlayerMovementComponentBase>()
                ->Version(1);
        }
        WasdPlayerMovementComponentBase::Reflect(context);
    }

    void WasdPlayerMovementComponent::OnInit()
    {
        ;
    }

    void WasdPlayerMovementComponent::OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        ;
    }

    void WasdPlayerMovementComponent::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        ;
    }

    WasdPlayerMovementComponentController::WasdPlayerMovementComponentController(WasdPlayerMovementComponent& parent)
        : WasdPlayerMovementComponentControllerBase(parent)
    {
        ;
    }

    void WasdPlayerMovementComponentController::OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        if (IsAutonomous())
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
        }
    }

    void WasdPlayerMovementComponentController::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        if (IsAutonomous())
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
        }
    }

    void WasdPlayerMovementComponentController::CreateInput(Multiplayer::NetworkInput& input, float deltaTime)
    {
        // Movement axis
        // Since we're on a keyboard, this adds a touch of an acceleration curve to the keyboard inputs
        // This is so that tapping the keyboard moves the virtual stick less than just holding it down
        m_forwardWeight = std::min<float>(m_forwardDown ? m_forwardWeight + cl_WasdStickAccel * deltaTime : 0.0f, 1.0f);
        m_leftWeight = std::min<float>(m_leftDown ? m_leftWeight + cl_WasdStickAccel * deltaTime : 0.0f, 1.0f);
        m_backwardWeight = std::min<float>(m_backwardDown ? m_backwardWeight + cl_WasdStickAccel * deltaTime : 0.0f, 1.0f);
        m_rightWeight = std::min<float>(m_rightDown ? m_rightWeight + cl_WasdStickAccel * deltaTime : 0.0f, 1.0f);

        // inputs for your own component always exist
        WasdPlayerMovementComponentNetworkInput* wasdInput = input.FindComponentInput<WasdPlayerMovementComponentNetworkInput>();

        wasdInput->m_forwardAxis = StickAxis(m_forwardWeight - m_backwardWeight);
        wasdInput->m_strafeAxis = StickAxis(m_leftWeight - m_rightWeight);

        // View Axis
        float camYaw = 0.0f; // Value [-pi, +pi]
        //const PlayerComponent::Autonomous* playerComponent = FindController<PlayerComponent::Autonomous>(GetHandle());
        //if (playerComponent == nullptr)
        //{
        //    // Try and get one from the primary entity (useful when we have multiple autonomous entities)
        //    playerComponent = FindController<PlayerComponent::Autonomous>(gNovaGame->GetNovaNetworkAgent().GetPrimaryPlayerEntity());
        //}
        //if (playerComponent != nullptr)
        //{
        //    AZ::Vector3 eulerAngs = AZ::Vector3::CreateZero();
        //    EBUS_EVENT_ID_RESULT(eulerAngs, playerComponent->GetCameraEnt(), AZ::TransformBus, GetLocalRotation);
        //    camYaw = eulerAngs.GetZ();
        //    // euler angles come back 0 <-> 2PI, but we want -PI <-> PI ( we require to wrap around to -PI after hitting PI is the range mapping )
        //    if (camYaw > AZ::Constants::Pi)
        //    {
        //        camYaw -= AZ::Constants::TwoPi;
        //    }
        //
        //    camYaw = camYaw / AZ::Constants::Pi; // Normalize to [-1,1]
        //}
        wasdInput->m_viewYaw = MouseAxis(camYaw);

        // Strafe input
        wasdInput->m_sprint = m_sprinting;
        wasdInput->m_jump = m_jumping;
        wasdInput->m_crouch = m_crouching;

        // Just a note for anyone who is super confused by this, ResetCount is a predictable network property, it gets set on the client through correction packets
        wasdInput->m_resetCount = GetNetworkTransformComponentController()->GetResetCount();
    }

    void WasdPlayerMovementComponentController::ProcessInput(Multiplayer::NetworkInput& input, float deltaTime)
    {
        // If the input reset count doesn't match the state's reset count it can mean two things:
        //  1) On the server: we were reset and we are now receiving inputs from the client for an old reset count 
        //  2) On the client: we were reset and we are replaying old inputs after being corrected
        // In both cases we don't want to process these inputs
        WasdPlayerMovementComponentNetworkInput* wasdInput = input.FindComponentInput<WasdPlayerMovementComponentNetworkInput>();
        if (wasdInput->m_resetCount != GetNetworkTransformComponentController()->GetResetCount())
        {
            return;
        }

        GetNetworkAnimationComponentController()->ModifyActiveAnimStates().SetBit(aznumeric_cast<AZStd::size_t>(CharacterAnimState::Sprinting), wasdInput->m_sprint);
        GetNetworkAnimationComponentController()->ModifyActiveAnimStates().SetBit(aznumeric_cast<AZStd::size_t>(CharacterAnimState::Jumping), wasdInput->m_jump);
        GetNetworkAnimationComponentController()->ModifyActiveAnimStates().SetBit(aznumeric_cast<AZStd::size_t>(CharacterAnimState::Crouching), wasdInput->m_crouch);

        // Update orientation
        const float fwdBack = wasdInput->m_forwardAxis;
        const float leftRight = wasdInput->m_strafeAxis;
        AZ::Quaternion newOrientation = AZ::Quaternion::CreateRotationZ(wasdInput->m_viewYaw * AZ::Constants::Pi);

        // Update velocity
        UpdateVelocity(*wasdInput);

        // Ensure any entities that we might interact with are properly synchronized to their rewind state
        {
            const AZ::Aabb entityStartBounds = AZ::Interface<AzFramework::IEntityBoundsUnion>::Get()->GetEntityLocalBoundsUnion(GetEntity()->GetId());
            const AZ::Aabb entityFinalBounds = entityStartBounds.GetTranslated(m_velocity);
            AZ::Aabb entitySweptBounds = entityStartBounds;
            entitySweptBounds.AddAabb(entityFinalBounds);
            Multiplayer::GetNetworkTime()->SyncEntitiesToRewindState(entitySweptBounds);
        }

        GetCharacterComponentController()->TryMoveWithVelocity(m_velocity, deltaTime);
    }

    void WasdPlayerMovementComponentController::UpdateVelocity(const WasdPlayerMovementComponentNetworkInput& wasdInput)
    {
        const float fwdBack = wasdInput.m_forwardAxis;
        const float leftRight = wasdInput.m_strafeAxis;

        // Note that we really want to be setting a velocity anim graph param so the character strafes correctly
        if (fwdBack < 0.0f)
        {
            SetSpeed(GetCharacterComponentController()->GetReverseSprintSpeed());
        }
        else
        {
            if (wasdInput.m_sprint)
            {
                SetSpeed(GetCharacterComponentController()->GetSprintSpeed());
            }
            else
            {
                SetSpeed(GetCharacterComponentController()->GetWalkSpeed());
            }
        }

        // Not moving?
        if (fwdBack == 0.0f && leftRight == 0.0f)
        {
            SetSpeed(0.0f);
            m_velocity = AZ::Vector3::CreateZero();
        }
        else
        {
            const float stickInputAngle = AZ::Atan2(leftRight, fwdBack);

            float currentHeading = GetNetworkTransformComponentController()->GetRotation().GetEulerRadians().GetZ();
            float targetHeading = currentHeading + stickInputAngle; // Update current heading with stick input angles

            targetHeading = NormalizeHeading(targetHeading);

            static const AZ::Vector3 fwd = AZ::Vector3::CreateAxisY();
            m_velocity = AZ::Quaternion::CreateRotationZ(targetHeading).TransformVector(fwd) * GetSpeed();
        }
    }

    float WasdPlayerMovementComponentController::NormalizeHeading(float heading) const
    {
        // Ensure a_Heading in range [-pi, +pi]
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

    void WasdPlayerMovementComponentController::OnPressed([[maybe_unused]] float value)
    {
        const StartingPointInput::InputEventNotificationId* inputId = StartingPointInput::InputEventNotificationBus::GetCurrentBusId();

        if (inputId == nullptr)
        {
            return;
        }

        if (*inputId == MoveFwdEventId)
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
    }

    void WasdPlayerMovementComponentController::OnReleased([[maybe_unused]] float value)
    {
        const StartingPointInput::InputEventNotificationId* inputId = StartingPointInput::InputEventNotificationBus::GetCurrentBusId();

        if (inputId == nullptr)
        {
            return;
        }

        if (*inputId == MoveFwdEventId)
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
    }

    void WasdPlayerMovementComponentController::OnHeld([[maybe_unused]] float value)
    {
        const StartingPointInput::InputEventNotificationId* inputId = StartingPointInput::InputEventNotificationBus::GetCurrentBusId();

        if (inputId == nullptr)
        {
            return;
        }
    }
}
