/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <Source/AutoGen/GemComponent.AutoComponent.h>

namespace MultiplayerSample
{
    class GemComponent
        : public GemComponentBase
    {
    public:
        AZ_MULTIPLAYER_COMPONENT(MultiplayerSample::GemComponent, s_gemComponentConcreteUuid, MultiplayerSample::GemComponentBase);

        static void Reflect(AZ::ReflectContext* context);

        void OnActivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
        void OnDeactivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;

    private:
        // Animate the gem on clients without spending network traffic. (The gem will not spin on the authority server.)
        void ClientAnimationTick();
        AZ::ScheduledEvent m_clientAnimationEvent{ [this]()
        {
            ClientAnimationTick();
        }, AZ::Name("GemComponent") };

        void OnNetworkLocationChanged(const AZ::Vector3& location);
        AZ::Event<AZ::Vector3>::Handler m_networkLocationHandler{ [this](const AZ::Vector3& location)
        {
            OnNetworkLocationChanged(location);
        } };

        AZ::Vector3 m_rootLocation = AZ::Vector3::CreateZero();
        AZ::TimeMs m_lifetime = AZ::Time::ZeroTimeMs;
    };

    class GemComponentController
        : public GemComponentControllerBase
    {
    public:
        explicit GemComponentController(GemComponent& parent);

        void OnActivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
        void OnDeactivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;

#if AZ_TRAIT_SERVER
        void HandleRPC_CollectedByPlayer(AzNetworking::IConnection* invokingConnection) override;
#endif
    };
}
