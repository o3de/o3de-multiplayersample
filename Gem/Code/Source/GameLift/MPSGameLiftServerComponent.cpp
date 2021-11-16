/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of
 * this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include "MPSGameLiftServerComponent.h"

#include <AzCore/Console/ILogger.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/EditContextConstants.inl>
#include <AzCore/Serialization/SerializeContext.h>

#include <AzNetworking/Framework/INetworking.h>
#include <AzFramework/Session/SessionConfig.h>

#include <Request/AWSGameLiftServerRequestBus.h>
#include <Multiplayer/MultiplayerConstants.h>

namespace MultiplayerSample
{
    using namespace AzNetworking;

    AZ_CVAR(bool, sv_manualBackfill, false, nullptr, AZ::ConsoleFunctorFlags::DontReplicate, "Whether to start manual backfill");

    void MPSGameLiftServerSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serialize = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serialize->Class<MPSGameLiftServerSystemComponent, AZ::Component>()->Version(0);

            if (AZ::EditContext* ec = serialize->GetEditContext())
            {
                ec->Class<MPSGameLiftServerSystemComponent>(
                      "MultiplayerSampleGameLiftServer", "System Component for the GameLift dedicated server")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("System"))
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true);
            }
        }
    }

    void MPSGameLiftServerSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("MultiplayerSampleGameLiftServerService"));
    }

    void MPSGameLiftServerSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("MultiplayerSampleGameLiftServerService"));
    }

    void MPSGameLiftServerSystemComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        required.push_back(AZ_CRC_CE("NetworkingService"));
        required.push_back(AZ_CRC_CE("MultiplayerService"));
        required.push_back(AZ_CRC_CE("AWSGameLiftServerService"));
    }

    void MPSGameLiftServerSystemComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        AZ_UNUSED(dependent);
    }

    void MPSGameLiftServerSystemComponent::Init()
    {
        m_backfillProcessTerminated = true;
        m_backfillComplete = true;
    }

    void MPSGameLiftServerSystemComponent::Activate()
    {
        AzFramework::SessionNotificationBus::Handler::BusConnect();

        AWSGameLift::AWSGameLiftServerRequestBus::Broadcast(&AWSGameLift::AWSGameLiftServerRequestBus::Events::NotifyGameLiftProcessReady);
    }

    void MPSGameLiftServerSystemComponent::Deactivate()
    {
        StopBackfillProcess();

        AzFramework::SessionNotificationBus::Handler::BusDisconnect();
    }

    bool MPSGameLiftServerSystemComponent::OnSessionHealthCheck()
    {
        return true;
    }

    bool MPSGameLiftServerSystemComponent::OnCreateSessionBegin(const AzFramework::SessionConfig& sessionConfig)
    {
        m_maxPlayer = sessionConfig.m_maxPlayer;

        return true;
    }

    void MPSGameLiftServerSystemComponent::OnCreateSessionEnd()
    {
        StartBackfillProcess();
    }

    bool MPSGameLiftServerSystemComponent::OnDestroySessionBegin()
    {
        StopBackfillProcess();

        return true;
    }

    void MPSGameLiftServerSystemComponent::OnDestroySessionEnd()
    {
        m_maxPlayer = 0;
    }

    void MPSGameLiftServerSystemComponent::OnUpdateSessionBegin(
        const AzFramework::SessionConfig& sessionConfig, const AZStd::string& updateReason)
    {
        AZ_UNUSED(sessionConfig);
        if (updateReason == "BACKFILL_FAILED" || updateReason == "BACKFILL_TIMED_OUT" || updateReason == "BACKFILL_CANCELLED")
        {
            m_backfillComplete = true;
            m_waitEvent.release();
        }
    }

    void MPSGameLiftServerSystemComponent::OnUpdateSessionEnd()
    {
        AZ_TracePrintf("MultiplayerSample", "Multiplayer sample server manual backfill has completed");

        m_backfillComplete = true;
    }

    void MPSGameLiftServerSystemComponent::StartBackfillProcess()
    {
        bool manualBackfill = false;
        const auto console = AZ::Interface<AZ::IConsole>::Get();
        if (console->GetCvarValue("sv_manualBackfill", manualBackfill) != AZ::GetValueResult::Success)
        {
            AZ_Error("GameLiftClientSystemComponent", false, "Failed to get sv_manualBackfill");
            return;
        }

        if (!manualBackfill || !m_backfillProcessTerminated)
        {
            // Either manual backfill is not enabled or there's a backfill request in progress.
            return;
        }

        // Launch a background thread to do the manual backfill periodically if there're empty slots in the session.
        AZ_TracePrintf("MultiplayerSample", "Start multiplayer sample server manual backfill");
        m_backfillProcessTerminated = false;
        m_backfillThread = AZStd::thread(AZStd::bind(&MPSGameLiftServerSystemComponent::BackfillProcess, this));
    }

    void MPSGameLiftServerSystemComponent::StopBackfillProcess()
    {
        bool manualBackfill = false;
        const auto console = AZ::Interface<AZ::IConsole>::Get();
        if (console->GetCvarValue("sv_manualBackfill", manualBackfill) != AZ::GetValueResult::Success)
        {
            AZ_Error("GameLiftClientSystemComponent", false, "Failed to get sv_manualBackfill");
            return;
        }

        if (!manualBackfill || m_backfillProcessTerminated)
        {
            // Either manual backfill is not enabled or there's no backfill request in progress.
            return;
        }

        m_backfillProcessTerminated = true;
        m_waitEvent.release();
        if (m_backfillThread.joinable())
        {
            m_backfillThread.join();
        }
    }

    void MPSGameLiftServerSystemComponent::BackfillProcess()
    {
        while (!m_backfillProcessTerminated)
        {
            m_waitEvent.try_acquire_for(AZStd::chrono::seconds(30));

            if (!m_backfillComplete || !HasEmptySlots())
            {
                // Either the previous backfill request hasn't been completed or there's no available slots for backfill
                continue;
            }

            AZ_TracePrintf("MultiplayerSample", "Initiate multiplayer sample server manual backfill");

            AZStd::string matchmakingTicketId = AZ::Uuid::CreateRandom().ToString<AZStd::string>(false, false);
            AZStd::vector<AWSGameLift::AWSGameLiftPlayer> players;
            m_backfillComplete = false;

            bool result = false;
            AWSGameLift::AWSGameLiftServerRequestBus::BroadcastResult(result,
                &AWSGameLift::AWSGameLiftServerRequestBus::Events::StartMatchBackfill, matchmakingTicketId, players);
            if (!result)
            {
                m_backfillComplete = true;
            }
        }
    }

    bool MPSGameLiftServerSystemComponent::HasEmptySlots()
    {
        AzNetworking::INetworking* networking = AZ::Interface<AzNetworking::INetworking>::Get();
        AzNetworking::INetworkInterface* clientNetworkInterface = networking->RetrieveNetworkInterface(AZ::Name(Multiplayer::MpNetworkInterfaceName));
        return clientNetworkInterface->GetConnectionSet().GetActiveConnectionCount() < m_maxPlayer;
    }
} // namespace MultiplayerSample
