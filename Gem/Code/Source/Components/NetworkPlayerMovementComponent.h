/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 * 
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <Source/AutoGen/NetworkPlayerMovementComponent.AutoComponent.h>
#include <Source/Components/NetworkAiComponent.h>
#include <StartingPointInput/InputEventNotificationBus.h>

namespace MultiplayerSample
{
    // Input Event Ids for Player Controls
    const StartingPointInput::InputEventNotificationId MoveFwdEventId("move_fwd");
    const StartingPointInput::InputEventNotificationId MoveBackEventId("move_back");
    const StartingPointInput::InputEventNotificationId MoveLeftEventId("move_left");
    const StartingPointInput::InputEventNotificationId MoveRightEventId("move_right");

    const StartingPointInput::InputEventNotificationId SprintEventId("sprint");
    const StartingPointInput::InputEventNotificationId JumpEventId("jump");
    const StartingPointInput::InputEventNotificationId CrouchEventId("crouch");

    const StartingPointInput::InputEventNotificationId LookLeftRightEventId("lookLeftRight");
    const StartingPointInput::InputEventNotificationId LookUpDownEventId("lookUpDown");

    const StartingPointInput::InputEventNotificationId ZoomInEventId("zoomIn");
    const StartingPointInput::InputEventNotificationId ZoomOutEventId("zoomOut");

    class NetworkPlayerMovementComponentController
        : public NetworkPlayerMovementComponentControllerBase
        , private StartingPointInput::InputEventNotificationBus::MultiHandler
    {
    public:
        NetworkPlayerMovementComponentController(NetworkPlayerMovementComponent& parent);

        //! NetworkPlayerMovementComponentControllerBase
        //! @{
        void OnActivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
        void OnDeactivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;

        void CreateInput(Multiplayer::NetworkInput& input, float deltaTime) override;
        void ProcessInput(Multiplayer::NetworkInput& input, float deltaTime) override;
        //! @}
    
    private:
        friend class NetworkAiComponentController;

        void UpdateVelocity(const NetworkPlayerMovementComponentNetworkInput& playerInput, float deltaTime);
        float NormalizeHeading(float heading) const;

        AZ::Vector3 GetSlopeHeading(float targetHeading, bool onGround) const;

        //! AZ::InputEventNotificationBus interface
        //! @{
        void OnPressed(float value) override;
        void OnReleased(float value) override;
        void OnHeld(float value) override;
        //! @}

        void UpdateAI();
        bool ShouldProcessInput() const;

        AZ::ScheduledEvent m_updateAI;
        NetworkAiComponentController* m_networkAiComponentController = nullptr;

        // Technically these values should never migrate hosts since they are maintained by the autonomous client
        // But due to how the stress test chaos monkey operates, it puppets these values on the server to mimic a client
        // This means these values can and will migrate between hosts (and lose any stored state)
        // We will need to consider moving these values to Authority to Server network properties if the design doesn't change
        float m_forwardWeight = 0.0f;
        float m_leftWeight = 0.0f;
        float m_backwardWeight = 0.0f;
        float m_rightWeight = 0.0f;

        float m_viewYaw = 0.0f;
        float m_viewPitch = 0.0f;

        bool m_forwardDown = false;
        bool m_leftDown = false;
        bool m_backwardDown = false;
        bool m_rightDown = false;
        bool m_sprinting = false;
        bool m_jumping = false;
        bool m_crouching = false;
        bool m_aiEnabled = false;
        bool m_wasOnGround = true;
        float m_gravity = -9.81f;
        float m_stepHeight = 0.1f;
        float m_radius = 0.3f;
    };
}
