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

#pragma once

#include <Source/AutoGen/WasdPlayerMovementComponent.AutoComponent.h>
#include <StartingPointInput/InputEventNotificationBus.h>

namespace MultiplayerSample
{
    // Input Event Ids for Player Controls
    const StartingPointInput::InputEventNotificationId MoveFwdEventId(AZ_CRC_CE("default"), "move_fwd");
    const StartingPointInput::InputEventNotificationId MoveBackEventId(AZ_CRC_CE("default"), "move_back");
    const StartingPointInput::InputEventNotificationId MoveLeftEventId(AZ_CRC_CE("default"), "move_left");
    const StartingPointInput::InputEventNotificationId MoveRightEventId(AZ_CRC_CE("default"), "move_right");

    const StartingPointInput::InputEventNotificationId SprintEventId(AZ_CRC_CE("default"), "sprint");
    const StartingPointInput::InputEventNotificationId JumpEventId(AZ_CRC_CE("default"), "jump");
    const StartingPointInput::InputEventNotificationId CrouchEventId(AZ_CRC_CE("default"), "crouch");

    const StartingPointInput::InputEventNotificationId LookLeftRightEventId(AZ_CRC_CE("default"), "lookLeftRight");
    const StartingPointInput::InputEventNotificationId LookUpDownEventId(AZ_CRC_CE("default"), "lookUpDown");

    class WasdPlayerMovementComponent
        : public WasdPlayerMovementComponentBase
    {
    public:
        AZ_MULTIPLAYER_COMPONENT(MultiplayerSample::WasdPlayerMovementComponent, s_wasdPlayerMovementComponentConcreteUuid, MultiplayerSample::WasdPlayerMovementComponentBase);

        static void Reflect(AZ::ReflectContext* context);

        void OnInit() override;
        void OnActivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
        void OnDeactivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
    };

    class WasdPlayerMovementComponentController
        : public WasdPlayerMovementComponentControllerBase
        , private StartingPointInput::InputEventNotificationBus::MultiHandler
    {
    public:
        WasdPlayerMovementComponentController(WasdPlayerMovementComponent& parent);

        void OnActivate(Multiplayer::EntityIsMigrating entityIsMigrating);
        void OnDeactivate(Multiplayer::EntityIsMigrating entityIsMigrating);

        void CreateInput(Multiplayer::NetworkInput& input, float deltaTime) override;
        void ProcessInput(Multiplayer::NetworkInput& input, float deltaTime) override;

    private:
        void UpdateVelocity(const WasdPlayerMovementComponentNetworkInput& wasdInput);
        float NormalizeHeading(float heading) const;

        //! AZ::InputEventNotificationBus interface
        //! @{
        void OnPressed(float value) override;
        void OnReleased(float value) override;
        void OnHeld(float value) override;
        //! @}

        AZ::Vector3 m_velocity = AZ::Vector3::CreateZero();

        float m_forwardWeight = 0.0f;
        float m_leftWeight = 0.0f;
        float m_backwardWeight = 0.0f;
        float m_rightWeight = 0.0f;

        bool  m_forwardDown = false;
        bool  m_leftDown = false;
        bool  m_backwardDown = false;
        bool  m_rightDown = false;
        bool  m_sprinting = false;
        bool  m_jumping = false;
        bool  m_crouching = false;
    };
}
