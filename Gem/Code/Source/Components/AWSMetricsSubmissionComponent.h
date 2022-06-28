/*
* Copyright (c) Contributors to the Open 3D Engine Project.
* For complete copyright and license terms please see the LICENSE at the root of this distribution.
*
* SPDX-License-Identifier: Apache-2.0 OR MIT
*
*/

#pragma once

#include <AzCore/Component/Component.h>
#include <AzCore/Component/TickBus.h>

using namespace Multiplayer;

namespace MultiplayerSample
{
    //! Component for submitting metrics to the AWS backend
    class AWSMetricsSubmissionComponent
        : public AZ::Component
        , public AZ::TickBus::Handler
    {
    public:
        AZ_COMPONENT(AWSMetricsSubmissionComponent, "{F16A5A09-C351-4862-8CCA-3E8419123538}", Component);

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);

        void Activate() override;
        void Deactivate() override;

        void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;

    private:
        //! Connect handlers to the player connection events for submitting client_join and client_leave metrics
        void RegisterPlayerConnectionMetrics();
        //! Submit the client_connection_count metrics
        void SubmitEndpointConnectionCountMetrics();

        ConnectionAcquiredEvent::Handler m_connectHandler;
        EndpointDisonnectedEvent::Handler m_disconnectHandler;

        //! Time interval for submitting metrics regularly.
        //! Default to 1 minute to reduce AWS costs and it's configurable via the Editor.
        float m_AWSMetricsSubmissionIntervalInSecHint=60.0;

        float m_interval = 0;
    };
}
