/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 * 
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <Source/AutoGen/NetworkSimplePlayerCameraComponent.AutoComponent.h>
#include <AzCore/Component/TickBus.h>

namespace MultiplayerSample
{
    class NetworkSimplePlayerCameraComponentController
        : public NetworkSimplePlayerCameraComponentControllerBase
        , private AZ::TickBus::Handler
    {
    public:
        NetworkSimplePlayerCameraComponentController(NetworkSimplePlayerCameraComponent& parent);

        void OnActivate(Multiplayer::EntityIsMigrating entityIsMigrating);
        void OnDeactivate(Multiplayer::EntityIsMigrating entityIsMigrating);

        float GetCameraYaw() const;
        float GetCameraPitch() const;
        float GetCameraRoll() const;

        float GetCameraYawPrevious() const;
        float GetCameraPitchPrevious() const;
        float GetCameraRollPrevious() const;

    private:
        //! AZ::TickBus interface
        //! @{
        void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;
        int GetTickOrder() override;
        //! @}

        AZ::Entity* m_activeCameraEntity = nullptr;
        bool m_aiEnabled = false;
    };
}
