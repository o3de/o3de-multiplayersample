/*
 * Copyright (c) Contributors to the Open 3D Engine Project
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AzCore/Component/TransformBus.h>
#include <AzCore/Serialization/EditContext.h>
#include <Components/NetworkTraceComponent.h>

namespace MultiplayerSample
{
    NetworkTraceComponent::NetworkTraceComponent()
        : m_serverPositionChanged([this](AZ::Vector3 serverPosition) {OnServerPositionChanged(serverPosition); })
        , m_serverTimeChanged([this](uint32_t serverTime) {OnServerTimeChanged(serverTime); })
        , m_updateTraceOverlay([this]() {UpdateTraceOverlay(); }, AZ::Name("UpdateTraceOverlay"))
    {
    }

    void NetworkTraceComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto* serializeContext = azrtti_cast<AZ::SerializeContext*>(context); serializeContext)
        {
            serializeContext->Class<NetworkTraceComponent, NetworkTraceComponentBase>()
                ->Version(1);
        }
        NetworkTraceComponentBase::Reflect(context);
    }

    void NetworkTraceComponent::OnInit()
    {
    }

    void NetworkTraceComponent::OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        if (IsNetEntityRoleAutonomous())
        {
            AZ::TickBus::Handler::BusConnect();

            m_previousClientPositions.set_capacity(60);
            m_previousServerPositions.set_capacity(60);

            ServerPositionAddEvent(m_serverPositionChanged);
            ServerTimeAddEvent(m_serverTimeChanged);

            m_updateTraceOverlay.Enqueue(AZ::TimeMs{ 100 }, true);
        }
    }

    void NetworkTraceComponent::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        AZ::TickBus::Handler::BusDisconnect();

        m_previousClientPositions.clear();
        m_previousServerPositions.clear();

        m_serverPositionChanged.Disconnect();
        m_serverTimeChanged.Disconnect();
    }

    void NetworkTraceComponent::OnTick([[maybe_unused]] float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time)
    {
        if (m_debugDisplay == nullptr)
        {
            AzFramework::DebugDisplayRequestBus::BusPtr debugDisplayBus;
            AzFramework::DebugDisplayRequestBus::Bind(debugDisplayBus, AzFramework::g_defaultSceneEntityDebugDisplayId);
            m_debugDisplay = AzFramework::DebugDisplayRequestBus::FindFirstHandler(debugDisplayBus);
        }

        const AZ::u32 stateBefore = m_debugDisplay->GetState();

        DrawTraceLine(m_previousClientPositions, AZ::Colors::Blue, "C");
        DrawTraceLine(m_previousServerPositions, AZ::Colors::Red, "S");

        m_debugDisplay->SetState(stateBefore);
    }

    void NetworkTraceComponent::DrawTraceLine(const AZStd::ring_buffer<PositionInTime>& data, const AZ::Color& color, const char* heading)
    {
        m_debugDisplay->SetColor(color);
        for (AZStd::size_t pointIndex = 1; pointIndex < data.size(); ++pointIndex)
        {
            const PositionInTime& prevPoint = data[pointIndex - 1];
            const PositionInTime& nextPoint = data[pointIndex];

            m_debugDisplay->DrawLine(prevPoint.m_position, nextPoint.m_position);
            char buffer[10] = {};
            azsprintf(buffer, "%lu", nextPoint.m_time);

            if (pointIndex < (data.size() - 1))
            {
                m_debugDisplay->DrawTextLabel(nextPoint.m_position, 0.7f, buffer);
            }
            else
            {
                m_debugDisplay->DrawTextLabel(nextPoint.m_position, 0.7f, heading);
            }
        }
    }

    void NetworkTraceComponent::OnServerPositionChanged(const AZ::Vector3& serverPosition)
    {
        m_latestServerPositionInTime.m_position = serverPosition;
    }

    void NetworkTraceComponent::OnServerTimeChanged(uint32_t serverTime)
    {
        m_latestServerPositionInTime.m_time = serverTime;
    }

    void NetworkTraceComponent::UpdateTraceOverlay()
    {
        if (m_latestServerPositionInTime.m_position.IsZero() == false)
        {
            if (m_latestServerPositionInTime.m_time != m_previousServerPositions.back().m_time)
            {
                m_previousServerPositions.push_back(m_latestServerPositionInTime);
                m_latestServerPositionInTime = { AZ::Vector3::CreateZero(), 0 };
            }
        }

        AZ::Vector3 entityPosition = AZ::Vector3::CreateZero();
        AZ::TransformBus::EventResult(entityPosition, GetEntityId(), &AZ::TransformBus::Events::GetWorldTranslation);
        if (entityPosition.IsZero() == false)
        {
            const uint32_t localTime = aznumeric_cast<uint32_t>(Multiplayer::GetMultiplayer()->GetNetworkTime()->GetHostFrameId());
            m_previousClientPositions.push_back({ entityPosition, localTime });
        }
    }

    NetworkTraceComponentController::NetworkTraceComponentController(NetworkTraceComponent& parent)
        : NetworkTraceComponentControllerBase(parent)
    {
    }

    void NetworkTraceComponentController::OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        if (GetOwner().IsNetEntityRoleAuthority())
        {
            AZ::TickBus::Handler::BusConnect();
        }
    }

    void NetworkTraceComponentController::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        if (GetOwner().IsNetEntityRoleAuthority())
        {
            AZ::TickBus::Handler::BusDisconnect();
        }
    }

    void NetworkTraceComponentController::OnTick([[maybe_unused]] float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time)
    {
        AZ::Vector3 entityPosition = AZ::Vector3::CreateZero();
        AZ::TransformBus::EventResult(entityPosition, GetOwner().GetEntityId(), &AZ::TransformBus::Events::GetWorldTranslation);

        ModifyServerPosition() = entityPosition;
        ModifyServerTime() = aznumeric_cast<uint32_t>(Multiplayer::GetMultiplayer()->GetNetworkTime()->GetHostFrameId());
    }
}
