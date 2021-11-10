/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of
 * this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/Component/Component.h>
#include <AzCore/std/parallel/binary_semaphore.h>
#include <AzFramework/Session/SessionNotifications.h>

namespace AzNetworking
{
    class INetworkInterface;
}

namespace MultiplayerSample
{
    class GameLiftServerSystemComponent
        : public AZ::Component
        , public AzFramework::SessionNotificationBus::Handler
    {
    public:
        AZ_COMPONENT(GameLiftServerSystemComponent, "{5768429C-65CE-47B2-854F-3D1BECEB9D0C}");

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

    protected:
        ////////////////////////////////////////////////////////////////////////
        // AZ::Component interface implementation
        void Init() override;
        void Activate() override;
        void Deactivate() override;

        ////////////////////////////////////////////////////////////////////////
        // AzFramework::SessionNotificationBus::Handler overrides
        bool OnSessionHealthCheck() override;
        bool OnCreateSessionBegin(const AzFramework::SessionConfig& sessionConfig) override;
        void OnCreateSessionEnd() override;
        bool OnDestroySessionBegin() override;
        void OnDestroySessionEnd() override;
        void OnUpdateSessionBegin(const AzFramework::SessionConfig& sessionConfig, const AZStd::string& updateReason) override;
        void OnUpdateSessionEnd() override;
        ////////////////////////////////////////////////////////////////////////

    private:
        void StartBackfillProcess();
        void StopBackfillProcess();
        void BackfillProcess();
        bool HasEmptySlots();

        AZStd::atomic<bool> m_backfillProcessTerminated;
        AZStd::thread m_backfillThread;
        AZStd::binary_semaphore m_waitEvent;

        AZStd::atomic<uint64_t> m_maxPlayer;
        AZStd::atomic<bool> m_backfillComplete;
    };
} // namespace MultiplayerSample
