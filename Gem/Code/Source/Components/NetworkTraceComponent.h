/*
 * Copyright (c) Contributors to the Open 3D Engine Project
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/Component/Component.h>
#include <AzCore/Component/TickBus.h>
#include <AzCore/std/containers/ring_buffer.h>
#include <AzFramework/Entity/EntityDebugDisplayBus.h>
#include <Source/AutoGen/NetworkTraceComponent.AutoComponent.h>

namespace MultiplayerSample
{
    //! @class NetworkTraceComponent
    //! @brief Traces out client and server lines on clients.
    class NetworkTraceComponent
        : public NetworkTraceComponentBase
        , public AZ::TickBus::Handler
    {
    public:
        AZ_MULTIPLAYER_COMPONENT(MultiplayerSample::NetworkTraceComponent, s_networkTraceComponentConcreteUuid, MultiplayerSample::NetworkTraceComponentBase);

        NetworkTraceComponent();
        static void Reflect(AZ::ReflectContext* context);

        void OnInit() override;
        void OnActivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
        void OnDeactivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;

        //! AZ::TickBus overrides.
        //! @{
        void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;
        //! }@

    private:
        AzFramework::DebugDisplayRequests* m_debugDisplay = nullptr;

        struct PositionInTime
        {
            AZ::Vector3 m_position;
            uint32_t m_time;
        };

        AZStd::ring_buffer<PositionInTime> m_previousClientPositions;
        AZStd::ring_buffer<PositionInTime> m_previousServerPositions;

        AZ::Event<AZ::Vector3>::Handler m_serverPositionChanged;
        void OnServerPositionChanged(const AZ::Vector3& serverPosition);

        AZ::Event<uint32_t>::Handler m_serverTimeChanged;
        void OnServerTimeChanged(uint32_t serverTime);

        PositionInTime m_latestServerPositionInTime = { AZ::Vector3::CreateZero(), 0 };

        AZ::ScheduledEvent m_updateTraceOverlay;
        void UpdateTraceOverlay();
        void DrawTraceLine(const AZStd::ring_buffer<PositionInTime>& data, const AZ::Color& color, const char* heading);
    };

    class NetworkTraceComponentController
        : public NetworkTraceComponentControllerBase
        , public AZ::TickBus::Handler
    {
    public:
        NetworkTraceComponentController(NetworkTraceComponent& parent);

        void OnActivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
        void OnDeactivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;

        //! AZ::TickBus overrides.
        //! @{
        void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;
        //! }@
    };
}
