/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <Source/AutoGen/RpcTesterComponent.AutoComponent.h>

namespace MultiplayerSample
{
    class RpcTesterComponent
        : public RpcTesterComponentBase
    {
        friend class RpcTesterComponentController;
    public:
        AZ_MULTIPLAYER_COMPONENT(MultiplayerSample::RpcTesterComponent, s_rpcTesterComponentConcreteUuid, MultiplayerSample::RpcTesterComponentBase);

        static void Reflect(AZ::ReflectContext* context);
        
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
        {
            required.push_back(AZ_CRC_CE("MaterialProviderService"));
            RpcTesterComponentBase::GetRequiredServices(required);
        }

        void OnActivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
        void OnDeactivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;

        void HandleRPC_TestPassed(AzNetworking::IConnection* invokingConnection) override;

    private:
        void RunTests();
        AZ::ScheduledEvent m_delayTestRun{ [this]()
        {
            RunTests();
        }, AZ::Name("RpcTesterComponent") };
    };


    class RpcTesterComponentController
        : public RpcTesterComponentControllerBase
    {
    public:
        explicit RpcTesterComponentController(RpcTesterComponent& parent);

        void OnActivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
        void OnDeactivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;

        void HandleRPC_AutonomousToAuthority(AzNetworking::IConnection* invokingConnection) override;
        void HandleRPC_AuthorityToAutonomous(AzNetworking::IConnection* invokingConnection) override;
        void HandleRPC_ServerToAuthority(AzNetworking::IConnection* invokingConnection) override;
    };
}
