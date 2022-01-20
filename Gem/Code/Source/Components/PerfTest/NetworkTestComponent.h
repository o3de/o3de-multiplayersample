/*
 * Copyright (c) Contributors to the Open 3D Engine Project
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/Component/Component.h>
#include <AzCore/Component/TickBus.h>

namespace MultiplayerSample
{
    //! @class NetworkTestComponent
    class NetworkTestComponent final
        : public AZ::Component
        , public AZ::TickBus::Handler
    {
    public:
        AZ_COMPONENT(MultiplayerSample::NetworkTestComponent, "{7FAA74C4-5A35-4602-95D9-E83DE9EC7B01}");

        static void Reflect(AZ::ReflectContext* context);

        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
        {
            provided.push_back(AZ_CRC_CE("NetBindService"));
        }

        //! AZ::Component overrides.
        //! @{
        void Activate() override;
        void Deactivate() override;
        //! }@

        //! AZ::TickBus overrides.
        //! @{
        void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;
        //! }@

    private:
        bool m_enableMotion = false;
        float m_oscillatorAmplitude = 1.f;
        float m_oscillatorSpeedFactor = 1.f;

        float m_accumulatedTime = 0.f;
        AZ::Vector3 m_startTranslation = AZ::Vector3::CreateZero();
    };
}
