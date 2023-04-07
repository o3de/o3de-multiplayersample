/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 * 
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <Source/Components/NetworkPlayerMovementComponent.h>
#include <Source/Components/NetworkWeaponsComponent.h>
#include <Source/Components/NetworkAiComponent.h>
#include <Multiplayer/Components/NetworkCharacterComponent.h>
#include <Source/Components/NetworkAnimationComponent.h>
#include <Source/Components/NetworkMatchComponent.h>
#include <Source/Components/NetworkSimplePlayerCameraComponent.h>
#include <Multiplayer/Components/NetworkTransformComponent.h>
#include <AzCore/Time/ITime.h>
#include <AzFramework/Components/CameraBus.h>
#include <AzFramework/Physics/SystemBus.h>
#include <AzFramework/Physics/PhysicsScene.h>
#include <AzFramework/Physics/CharacterBus.h>
#include <AzFramework/Physics/Common/PhysicsTypes.h>
#include <AzFramework/Physics/Components/SimulatedBodyComponentBus.h>
#include <PhysX/CharacterGameplayBus.h>
#include <PhysX/CharacterControllerBus.h>
#include <GameplayEffectsNotificationBus.h>

namespace MultiplayerSample
{
    AZ_CVAR(float, cl_WasdStickAccel, 5.0f, nullptr, AZ::ConsoleFunctorFlags::Null, "The linear acceleration to apply to WASD inputs to simulate analog stick controls");
    AZ_CVAR(float, cl_AimStickScaleZ, 0.025f, nullptr, AZ::ConsoleFunctorFlags::Null, "The scaling to apply to aim and view adjustments");
    AZ_CVAR(float, cl_AimStickScaleX, 0.0125f, nullptr, AZ::ConsoleFunctorFlags::Null, "The scaling to apply to aim and view adjustments");

    /*
     * @cl_MaxMouseDelta should be large enough to contain the sum of mouse deltas across frames
     * between NetworkPlayerMovementComponentController::CreateInput() calls,
     * which happens at the frequency set by @cl_InputRateMs (33 times per second by default).
     *
     * The total value is then quantized using type @MouseAxis in "MultiplayerSampleTypes.h", for example:
     *     using MouseAxis = AzNetworking::QuantizedValues<1, 2, -1, 1>;
     * (The four numbers within the template parameters above read as: for a single value, spend 2 bytes to quantize a value between -1 and 1.)
     *
     * Keep the ranges and the precision of QuantizedValues and float types in mind when modifying mouse input configuration values.
     */
    AZ_CVAR(float, cl_MaxMouseDelta, 128.0f, nullptr, AZ::ConsoleFunctorFlags::Null, "The sum of mouse deltas will be clamped to this maximum");

#if AZ_TRAIT_CLIENT
    AZ_CVAR(bool, mps_botMode, false, nullptr, AZ::ConsoleFunctorFlags::Null, "If true, enable bot (AI) mode for client.");
    AZ_CVAR(float, mps_botMinInterval, 500.0f, nullptr, AZ::ConsoleFunctorFlags::Null, "The minimum amount of time between bot control updates");
    AZ_CVAR(float, mps_botMaxInterval, 9500.0f, nullptr, AZ::ConsoleFunctorFlags::Null, "The maximum amount of time between bot control updates");
#endif

    NetworkPlayerMovementComponentController::NetworkPlayerMovementComponentController(NetworkPlayerMovementComponent& parent)
        : NetworkPlayerMovementComponentControllerBase(parent)
#if AZ_TRAIT_SERVER		
        , m_updateAI{ [this] { UpdateAI(); }, AZ::Name{ "MovementControllerAi" } }
#endif
#if AZ_TRAIT_CLIENT
    , m_updateLocalBot{ [this] { UpdateLocalBot(); }, AZ::Name{ "MovementControllerLocalBot" } }
#endif
    {
        ;
    }

    void NetworkPlayerMovementComponentController::OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
#if AZ_TRAIT_SERVER
        NetworkAiComponent* networkAiComponent = GetParent().GetNetworkAiComponent();
        m_aiEnabled = (networkAiComponent != nullptr) ? networkAiComponent->GetEnabled() : false;
        if (m_aiEnabled)
        {
            m_updateAI.Enqueue(AZ::TimeMs{ 0 }, true);
            m_networkAiComponentController = GetNetworkAiComponentController();
        }
#endif

#if AZ_TRAIT_CLIENT
        if (IsNetEntityRoleAutonomous())
        {
            if (mps_botMode)
            {
                m_updateLocalBot.Enqueue(AZ::TimeMs{ 0 }, true);
            }
            else
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
#endif

        // During activation the character controller is not created yet.
        // Connect to CharacterNotificationBus to listen when it's activated after creation.
        Physics::CharacterNotificationBus::Handler::BusConnect(GetEntityId());

        AzPhysics::SimulatedBody* worldBody = nullptr;
        AzPhysics::SimulatedBodyComponentRequestsBus::EventResult(worldBody, GetEntityId(), &AzPhysics::SimulatedBodyComponentRequests::GetSimulatedBody);
        if (worldBody)
        {
            if (auto* sceneInterface = AZ::Interface<AzPhysics::SceneInterface>::Get())
            {
                m_gravity = sceneInterface->GetGravity(worldBody->m_sceneOwner).GetZ();
            }
        }
    }

    void NetworkPlayerMovementComponentController::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
#if AZ_TRAIT_CLIENT
        if (IsNetEntityRoleAutonomous() && !mps_botMode)
        {
            StartingPointInput::InputEventNotificationBus::MultiHandler::BusDisconnect();
        }
#endif
    }

    void NetworkPlayerMovementComponentController::OnCharacterActivated([[maybe_unused]] const AZ::EntityId& entityId)
    {
        // Wait until the character is activated before requesting its parameters.
        Physics::CharacterRequestBus::EventResult(m_stepHeight, GetEntityId(), &Physics::CharacterRequestBus::Events::GetStepHeight);
        PhysX::CharacterControllerRequestBus::EventResult(m_radius, GetEntityId(), &PhysX::CharacterControllerRequestBus::Events::GetRadius);
        PhysX::CharacterGameplayRequestBus::EventResult(
            m_gravityMultiplier, GetEntityId(), &PhysX::CharacterGameplayRequestBus::Events::GetGravityMultiplier);

        Physics::CharacterNotificationBus::Handler::BusDisconnect();
    }

    void NetworkPlayerMovementComponentController::CreateInput(Multiplayer::NetworkInput& input, float deltaTime)
    {
        // Inputs for your own component always exist
        NetworkPlayerMovementComponentNetworkInput* playerInput = input.FindComponentInput<NetworkPlayerMovementComponentNetworkInput>();

        // Check current game-play state
        INetworkMatch* networkMatchComponent = AZ::Interface<INetworkMatch>::Get();
        if (networkMatchComponent && (networkMatchComponent->PlayerActionsAllowed() != AllowedPlayerActions::None))
        {
            // View Axis are clamped and brought into the -1,1 range for transport across the network.
            // These are set if the player actions allow for rotation and/or all movement.
            playerInput->m_viewYaw = MouseAxis(AZStd::clamp<float>(m_viewYaw, -cl_MaxMouseDelta, cl_MaxMouseDelta) / cl_MaxMouseDelta);
            playerInput->m_viewPitch = MouseAxis(AZStd::clamp<float>(m_viewPitch, -cl_MaxMouseDelta, cl_MaxMouseDelta) / cl_MaxMouseDelta);
        }

        // reset accumulated amounts
        m_viewYaw = 0.f;
        m_viewPitch = 0.f;

        if (networkMatchComponent && (networkMatchComponent->PlayerActionsAllowed() == AllowedPlayerActions::All))
        {
            // Check if the user requested to toggle sprint-state
            if (m_toggleSprint)
            {
                m_sprinting = !m_sprinting;
                m_toggleSprint = false;
            }

            // Movement axis
            // Since we're on a keyboard, this adds a touch of an acceleration curve to the keyboard inputs
            // This is so that tapping the keyboard moves the virtual stick less than just holding it down
            m_forwardWeight = std::min<float>(m_forwardDown ? m_forwardWeight + cl_WasdStickAccel * deltaTime : 0.0f, 1.0f);
            m_leftWeight = std::min<float>(m_leftDown ? m_leftWeight + cl_WasdStickAccel * deltaTime : 0.0f, 1.0f);
            m_backwardWeight = std::min<float>(m_backwardDown ? m_backwardWeight + cl_WasdStickAccel * deltaTime : 0.0f, 1.0f);
            m_rightWeight = std::min<float>(m_rightDown ? m_rightWeight + cl_WasdStickAccel * deltaTime : 0.0f, 1.0f);

            playerInput->m_forwardAxis = StickAxis(m_forwardWeight - m_backwardWeight);
            playerInput->m_strafeAxis = StickAxis(m_leftWeight - m_rightWeight);

            playerInput->m_sprint = m_sprinting && (playerInput->m_forwardAxis > 0.0f); // Only sprint if we're moving forward
            playerInput->m_jump = m_jumping;
            playerInput->m_crouch = m_crouching;
        }
        else
        {
            // Don't set m_forward/left/right/backDown to 0, instead just 0 out the net-input by hand.
            // This way players can start the round hot out of the gate.
            // Keep their finger on the 'W' (forward) key, and instantly start
            // running when the round starts instead of having to release the 'W' key and pressing it again.
            playerInput->m_forwardAxis = StickAxis(0);
            playerInput->m_strafeAxis = StickAxis(0);

            playerInput->m_sprint = false;
            playerInput->m_jump = false;
            playerInput->m_crouch = false;
        }

        // reset jumping until next press. We only track when the jump is initially pressed, not that it's being held.
        m_jumping = false;

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

        NetworkWeaponsComponentNetworkInput* weaponInput = input.FindComponentInput<NetworkWeaponsComponentNetworkInput>();
        if ((weaponInput != nullptr) && weaponInput->m_firing.AnySet())
        {
            // Note that weaponInput is not guaranteed to exist, so we have to check for nullptr
            // Don't allow sprinting when the character is trying to shoot
            playerInput->m_sprint = false;
        }

        const bool wasOnGround = GetWasOnGround();
        bool onGround = GetOnGround();

        // Update the "on ground" state for the character.
        PhysX::CharacterGameplayRequestBus::EventResult(onGround, GetEntityId(), &PhysX::CharacterGameplayRequestBus::Events::IsOnGround);
        SetOnGround(onGround);

        // Track timers for how recently it's been since the player was on the ground and how recently they pressed the jump button.
        // These will be compared against "slop factors" to allow for a little bit of leniency in jumping to make it feel more reactive.
        SetSecondsSinceOnGround(onGround ? 0.0f : (GetSecondsSinceOnGround() + deltaTime));
        SetSecondsSinceJumpRequest(playerInput->m_jump ? 0.0f : (GetSecondsSinceJumpRequest() + deltaTime));

        // Update orientation
        AZ::Vector3 aimAngles = GetNetworkSimplePlayerCameraComponentController()->GetAimAngles();
        aimAngles.SetZ(NormalizeHeading(aimAngles.GetZ() - playerInput->m_viewYaw * cl_AimStickScaleZ * cl_MaxMouseDelta));
        aimAngles.SetX(NormalizeHeading(aimAngles.GetX() - playerInput->m_viewPitch * cl_AimStickScaleX * cl_MaxMouseDelta));
        aimAngles.SetX(NormalizeHeading(AZ::GetClamp(aimAngles.GetX(), -AZ::Constants::QuarterPi * 1.5f, AZ::Constants::QuarterPi * 1.5f)));
        GetNetworkSimplePlayerCameraComponentController()->SetAimAngles(aimAngles);

        const AZ::Quaternion newOrientation = AZ::Quaternion::CreateRotationZ(aimAngles.GetZ());
        GetEntity()->GetTransform()->SetLocalRotationQuaternion(newOrientation);

        // Update velocity
        bool jumpTriggered = false;
        bool movingDownward = false;
        UpdateVelocity(*playerInput, deltaTime, jumpTriggered, movingDownward);

        // absolute velocity is based on velocity generated by the player and other sources
        const AZ::Vector3 absoluteVelocity = GetVelocityFromExternalSources() + GetSelfGeneratedVelocity();

        GetNetworkCharacterComponentController()->TryMoveWithVelocity(absoluteVelocity, deltaTime);

        // If a jump was triggered, reset our jump request time to our "slop threshold" so that we don't double-count the jump request
        // if we land too quickly.
        if (jumpTriggered)
        {
            SetSecondsSinceJumpRequest(GetJumpPressQueuedSeconds());
        }

        // Tell the camera whether or not we're sprinting
        GetNetworkSimplePlayerCameraComponentController()->SetSprintMode(playerInput->m_sprint);
        GetNetworkAnimationComponentController()->ModifyActiveAnimStates().SetBit(aznumeric_cast<uint32_t>(CharacterAnimState::Sprinting), playerInput->m_sprint);
        GetNetworkAnimationComponentController()->ModifyActiveAnimStates().SetBit(aznumeric_cast<uint32_t>(CharacterAnimState::Crouching), playerInput->m_crouch);

        // The Landing anim state will automatically turn off after it's triggered
        GetNetworkAnimationComponentController()->ModifyActiveAnimStates().SetBit(aznumeric_cast<uint32_t>(CharacterAnimState::Landing), onGround && !wasOnGround && !jumpTriggered);

        // Always set/clear the jump state every tick or you might get ghost jump animations.
        // We only set it on the tick where the jump is first triggered, not for the entire jump.
        GetNetworkAnimationComponentController()->ModifyActiveAnimStates().SetBit(aznumeric_cast<uint32_t>(CharacterAnimState::Jumping), jumpTriggered);

        // Set whether or not we're currently falling.
        GetNetworkAnimationComponentController()->ModifyActiveAnimStates().SetBit(aznumeric_cast<uint32_t>(CharacterAnimState::Falling), !onGround);

        // If we're still on the ground, then zero out our velocity from external forces
        // This prevents us from sliding along the ground after we land
        PhysX::CharacterGameplayRequestBus::EventResult(onGround, GetEntityId(), &PhysX::CharacterGameplayRequestBus::Events::IsOnGround);
        if (onGround)
        {
            SetVelocityFromExternalSources(AZ::Vector3::CreateZero());
        }

        // At the end, track whether or not we were on the ground for this input so we can compare states when processing the next input.
        SetWasOnGround(onGround);
    }

#if AZ_TRAIT_SERVER
    void NetworkPlayerMovementComponentController::HandleApplyImpulse([[maybe_unused]] AzNetworking::IConnection* connection, const AZ::Vector3& impulse, const bool& external)
    {
        if (external)
        {
            const AZ::Vector3 newVelocity = GetVelocityFromExternalSources() + impulse;
            SetVelocityFromExternalSources(newVelocity);
        }
        else
        {
            const AZ::Vector3 newVelocity = GetSelfGeneratedVelocity() + impulse;
            SetSelfGeneratedVelocity(newVelocity);
        }
    }

    void NetworkPlayerMovementComponentController::HandleSetVelocity([[maybe_unused]] AzNetworking::IConnection* connection, const AZ::Vector3& velocity, const bool& external)
    {
        if (external)
        {
            SetVelocityFromExternalSources(velocity);
        }
        else
        {
            SetSelfGeneratedVelocity(velocity);
        }
    }
#endif

    void NetworkPlayerMovementComponentController::UpdateVelocity(const NetworkPlayerMovementComponentNetworkInput& playerInput, float deltaTime, bool& jumpTriggered, bool& movingDownward)
    {
        AZ::Vector3 velocityFromExternalSources = GetVelocityFromExternalSources(); // non-player generated (jump pads, explosions etc.)
        AZ::Vector3 selfGeneratedVelocity = GetSelfGeneratedVelocity(); // player generated

        const float secondsSinceOnGround = GetSecondsSinceOnGround();
        const float secondsSinceJumpRequest = GetSecondsSinceJumpRequest();

        const bool onGround = GetOnGround();

        if (onGround)
        {
            // Reset our jumping state if we're on the ground.
            if (GetIsJumping())
            {
                SetIsJumping(false);
            }

            // If we're on the ground, we should never have velocities pushing us into the ground.
            if (selfGeneratedVelocity.GetZ() <= 0.0f)
            {
                selfGeneratedVelocity.SetZ(0.0f);
            }
        }
        else
        {
            // If we're not on the ground, apply gravity.
            // NOTE: We do this *before* trying to trigger a jump so that the jump can overwrite the velocity and not have
            // gravity applied on the first tick of the jump.
            selfGeneratedVelocity.SetZ(selfGeneratedVelocity.GetZ() + m_gravity * m_gravityMultiplier * deltaTime);
        }

        // Ideally, the way velocities would work below is that there would be a single player velocity tracking the player's
        // current velocity, and pressing the jump button and getting external sources would just add an impulse into the player's
        // velocity, which then would get ticked back down by gravity over time.
        // However, the animation graph is expecting the player-generated velocity to be tracked separately from any external sources,
        // so at least for now, we'll keep them as separate velocities.
        // The one place where this is a problem is in how to make gravity work, as seen below. It's not actually solved correctly
        // right now, but until something uses the external sources velocity, it's hard to tell whether or not it needs to be fixed
        // in a better way.

        // If we're not currently jumping, see if we should trigger a jump.
        if (!GetIsJumping())
        {
            // We can trigger a jump as long as we aren't currently falling
            if (selfGeneratedVelocity.GetZ() <= 0.0f)
            {
                // We can trigger a jump as long as we *were* on the ground in the last "slop factor" fractions of a second
                if (secondsSinceOnGround <= GetJumpOnGroundQueuedSeconds())
                {
                    // We can trigger a jump as long as we pressed the jump button in the last "slop factor" fractions of a second
                    if (secondsSinceJumpRequest <= GetJumpPressQueuedSeconds())
                    {
                        // We're jumping, so set the upwards velocity necessary to reach our desired jump height.
                        // Note that we're only setting Z velocity because the XY velocity components can be changed by the player
                        // even while in midair.
                        const float initialJumpVelocity = AZ::Sqrt(2.0f * (-m_gravity * m_gravityMultiplier) * GetMaxJumpHeight());
                        selfGeneratedVelocity.SetZ(initialJumpVelocity);
                        jumpTriggered = true;
                        SetIsJumping(true);

                        GameplayEffectsNotificationBus::Broadcast(&GameplayEffectsNotificationBus::Events::OnPositionalEffect,
                            SoundEffect::PlayerExertion, GetEntity()->GetTransform()->GetWorldTranslation());
                    }
                }
            }
        }

        // External sources can add velocity, but get clamped to 0 so that they never add negative velocity, since 
        // the selfGeneratedVelocity already fully accounts for gravity so that the player can fall. 
        // If we don't clamp it, we'll get too much gravity influence on the player.
        // Note that because we're applying gravity to the external source as well as to the self-generated velocity, 
        // we're double-counting gravity's influence. This will probably make the external source velocities a bit harder
        // to tune.
        velocityFromExternalSources.SetZ(
            AZStd::max(0.0f, velocityFromExternalSources.GetZ() + m_gravity * m_gravityMultiplier * deltaTime));

        // Now that we've got the vertical velocity from jumps / external sources / gravity accounted for, let's calculate
        // the player horizontal movement velocity.

        const float fwdBack = playerInput.m_forwardAxis;
        const float leftRight = playerInput.m_strafeAxis;

        // TODO break out into air/water/ground speeds
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

        // If the player isn't trying to move, set the self-generated XY velocity to 0.
        // If they *are* trying to move, set a velocity based on the XY movement direction requested and the Z direction based
        // on the slope of the ground directly ahead vs what's currently under the player.
        if (fwdBack != 0.0f || leftRight != 0.0f)
        {
            const float stickInputAngle = AZ::Atan2(leftRight, fwdBack);
            const float currentHeading = GetNetworkTransformComponentController()->GetRotation().GetEulerRadians().GetZ();
            const float targetHeadingAngleRadians = NormalizeHeading(currentHeading + stickInputAngle);
            
            // Using the unit vector from GetSlopeHeading that provides the direction of movement, multiply by speed
            // to get the moveVelocity.
            const AZ::Vector3 moveVelocity = GetSlopeHeading(targetHeadingAngleRadians) * speed;

            // Immediately switch to the newly-requested XY movement direction, even if in midair.
            // We've chosen not to apply acceleration / deceleration.
            selfGeneratedVelocity.SetX(moveVelocity.GetX());
            selfGeneratedVelocity.SetY(moveVelocity.GetY());

            // If we're jumping, use the jumping/falling Z velocity. If we're on the ground or falling not from a jump, then we'll
            // add in any movement velocity. This will be 0 if there's no ground near us, but it will be in the direction of the ground
            // if we're next to some ground that's within a step height up or down.
            // Note that we can't just check for "on ground" here, because in the case of moving down a ramp or small steps, we might
            // actually be off the ground a little bit and need to correct our movement downward to get back onto the ground.
            if (!GetIsJumping())
            {
                selfGeneratedVelocity.SetZ(selfGeneratedVelocity.GetZ() + moveVelocity.GetZ());

                // If we're not jumping and we have a downwards velocity, track that we're deliberately moving downward so that
                // we can distinguish this state from arbitrary falling.
                if (moveVelocity.GetZ() < 0.0f)
                {
                    movingDownward = true;
                }
            }
        }
        else
        {
            // instant deceleration for now
            selfGeneratedVelocity.SetX(0.f);
            selfGeneratedVelocity.SetY(0.f);
        }

        SetVelocityFromExternalSources(velocityFromExternalSources);
        SetSelfGeneratedVelocity(selfGeneratedVelocity);
    }

    AZ::Vector3 NetworkPlayerMovementComponentController::GetSlopeHeading(float headingAngleRadians) const
    {
        // Returns a unit vector pointing in the direction that the player is moving.

        // Start with a direction vector in the XY plane.
        const AZ::Vector3 fwd = AZ::Quaternion::CreateRotationZ(headingAngleRadians).TransformVector(AZ::Vector3::CreateAxisY());

        // The origin is set to the bottom of the player, not the center.
        const AZ::Vector3 origin = GetEntity()->GetTransform()->GetWorldTranslation();
        constexpr float forwardEpsilon = 0.01f;
        constexpr float heightEpsilon = 0.01f;

        // Raycast straight down in front of the player by a tiny amount (forwardEpsilon) starting at the step height plus an epsilon
        // and ending at negative step height plus an epsilon. This will tell us if there's any surface directly in front of the player
        // within the step height up or down. If so, we'll use that to calculate the Z direction.
        const AZ::Vector3 start = origin + fwd * (m_radius + forwardEpsilon) + AZ::Vector3(0.f, 0.f, m_stepHeight + heightEpsilon);

        AzPhysics::RayCastRequest request;
        request.m_start = start;
        request.m_direction = AZ::Vector3::CreateAxisZ(-1.f);
        request.m_distance = (m_stepHeight + heightEpsilon) * 2.f;
        request.m_queryType = AzPhysics::SceneQuery::QueryType::Static;

        AzPhysics::SceneQueryHits result;
        if (auto* sceneInterface = AZ::Interface<AzPhysics::SceneInterface>::Get())
        {
            if (AzPhysics::SceneHandle sceneHandle = sceneInterface->GetSceneHandle(AzPhysics::DefaultPhysicsSceneName);
                sceneHandle != AzPhysics::InvalidSceneHandle)
            {
                result = sceneInterface->QueryScene(sceneHandle, &request);
            }
        }

        // If we've found a surface in front of us that's within the step height in size in either direction, then we'll create a vector
        // from the current bottom of the player to that new location so that our heading direction accounts for the Z slope.
        if (result && result.m_hits[0].IsValid())
        {
            // we use epsilon here to avoid the case where we are pushing up against an object and become slightly
            // elevated
            if (result.m_hits[0].m_position.GetZ() < (origin.GetZ() - heightEpsilon))
            {
                const AZ::Vector3 delta = result.m_hits[0].m_position - origin;
                return delta.GetNormalized();
            }
        }
        return fwd;
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
            m_toggleSprint = true;
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
            // Accumulate input to be processed in CreateInput().
            // In-between two CreateInput() calls, multiple series of presses and holds can occur, accumulate all of them,
            // otherwise we will drop some of the input data by only including the last press and hold combination.
            m_viewYaw += value;
        }
        else if (*inputId == LookUpDownEventId)
        {
            m_viewPitch += value;
        }
    }

    void NetworkPlayerMovementComponentController::OnReleased([[maybe_unused]]float value)
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
        else if (*inputId == JumpEventId)
        {
            m_jumping = false;
        }
        else if (*inputId == CrouchEventId)
        {
            m_crouching = false;
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
            // accumulate input to be processed in CreateInput()
            m_viewYaw += value;
        }
        else if (*inputId == LookUpDownEventId)
        {
            // accumulate input to be processed in CreateInput()
            m_viewPitch += value;
        }
    }

#if AZ_TRAIT_SERVER
    void NetworkPlayerMovementComponentController::UpdateAI()
    {
        float deltaTime = static_cast<float>(m_updateAI.TimeInQueueMs()) / 1000.f;
        if (m_networkAiComponentController != nullptr)
        {
            m_networkAiComponentController->TickMovement(*this, deltaTime);
        }
    }
#endif

#if AZ_TRAIT_CLIENT
    void NetworkPlayerMovementComponentController::UpdateLocalBot()
    {
        float deltaTimeMs = static_cast<float>(m_updateLocalBot.TimeInQueueMs());
        m_botRemainingTime -= deltaTimeMs;

        if (m_botRemainingTime <= 0)
        {
            // Determine a new directive after 500 to 9500 ms
            m_botRemainingTime = (m_botLcg.GetRandomFloat() * (mps_botMaxInterval - mps_botMinInterval) + mps_botMinInterval);
            m_botTurnRate = (1.f / m_botRemainingTime);

            // Randomize new target yaw and pitch and compute the delta from the current yaw and pitch respectively
            m_botTargetYawDelta = -m_viewYaw + (m_botLcg.GetRandomFloat() * 2.f - 1.f);
            m_botTargetPitchDelta = -m_viewPitch + (m_botLcg.GetRandomFloat() - 0.5f);

            // Randomize the action and strafe direction (used only if we decide to strafe)
            m_botAction = static_cast<Action>(m_botLcg.GetRandom() % static_cast<int>(Action::COUNT));
            m_botStrafingRight = static_cast<bool>(m_botLcg.GetRandom() % 2);
        }

        // Translate desired motion into inputs

        // Interpolate the current view yaw and pitch values towards the desired values
        m_viewYaw += m_botTurnRate * deltaTimeMs * m_botTargetPitchDelta;
        m_viewPitch += m_botTurnRate * deltaTimeMs * m_botTargetPitchDelta;

        // Reset keyboard movement inputs decided on the previous frame
        m_forwardDown = false;
        m_backwardDown = false;
        m_leftDown = false;
        m_rightDown = false;
        m_sprinting = false;
        m_jumping = false;
        m_crouching = false;

        switch (m_botAction)
        {
        case Action::Default:
            m_forwardDown = true;
            break;
        case Action::Sprinting:
            m_forwardDown = true;
            m_sprinting = true;
            break;
        case Action::Jumping:
            m_forwardDown = true;
            m_jumping = true;
            break;
        case Action::Crouching:
            m_forwardDown = true;
            m_crouching = true;
            break;
        case Action::Strafing:
            if (m_botStrafingRight)
            {
                m_rightDown = true;
            }
            else
            {
                m_leftDown = true;
            }
            break;
        default:
            break;
        }
    }
#endif
} // namespace MultiplayerSample
