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

#include <Source/AutoGen/SimplePlayerCameraComponent.AutoComponent.h>
#include <AzCore/Component/TickBus.h>

namespace MultiplayerSample
{
    class SimplePlayerCameraComponentController
        : public SimplePlayerCameraComponentControllerBase
        , private AZ::TickBus::Handler
    {
    public:
        SimplePlayerCameraComponentController(SimplePlayerCameraComponent& parent);

        void OnActivate(Multiplayer::EntityIsMigrating entityIsMigrating);
        void OnDeactivate(Multiplayer::EntityIsMigrating entityIsMigrating);

        float GetCameraYaw() const;
        float GetCameraPitch() const;
        float GetCameraRoll() const;

    private:
        //! AZ::TickBus interface
        //! @{
        void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;
        int GetTickOrder() override;
        //! @}

        AZ::Entity* m_activeCameraEntity = nullptr;
    };
}
