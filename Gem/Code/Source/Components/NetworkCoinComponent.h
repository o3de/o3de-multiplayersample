/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <Source/AutoGen/NetworkCoinComponent.AutoComponent.h>

namespace MultiplayerSample
{
    class NetworkCoinComponent
        : public NetworkCoinComponentBase
    {
    public:
        AZ_MULTIPLAYER_COMPONENT(MultiplayerSample::NetworkCoinComponent, s_networkCoinComponentConcreteUuid, MultiplayerSample::NetworkCoinComponentBase);

        static void Reflect(AZ::ReflectContext* context);
        
        void OnActivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
        void OnDeactivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;

    protected:
        // Animate the coin on clients without spending network traffic. (The coin will not spin on the authority server.)
        void ClientAnimationTick();
        AZ::ScheduledEvent m_clientAnimationEvent{ [this]()
        {
            ClientAnimationTick();
        }, AZ::Name("NetworkCoinComponent") };

        void OnNetworkLocationChanged(const AZ::Vector3& location);
        AZ::Event<AZ::Vector3>::Handler m_networkLocationHandler{ [this](AZ::Vector3 location)
        {
            OnNetworkLocationChanged(location);
        } };

        AZ::Vector3 m_rootLocation = AZ::Vector3::CreateZero();
        AZ::TimeMs m_lifetime = AZ::Time::ZeroTimeMs;
    };

    class NetworkCoinComponentController
        : public NetworkCoinComponentControllerBase
    {
    public:
        explicit NetworkCoinComponentController(NetworkCoinComponent& parent);

        void OnActivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
        void OnDeactivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;

        void HandleCollectedByPlayer(AzNetworking::IConnection* invokingConnection) override;
    };
}
