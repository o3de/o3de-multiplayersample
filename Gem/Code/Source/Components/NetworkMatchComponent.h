/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/EBus/ScheduledEvent.h>
#include <Source/AutoGen/NetworkMatchComponent.AutoComponent.h>

namespace MultiplayerSample
{
    class NetworkMatchComponent
        : public NetworkMatchComponentBase
    {
    public:
        AZ_MULTIPLAYER_COMPONENT(MultiplayerSample::NetworkMatchComponent, s_networkMatchComponentConcreteUuid, MultiplayerSample::NetworkMatchComponentBase);

        static void Reflect(AZ::ReflectContext* context);

        void OnInit() override {};
        void OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating) override {};
        void OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating) override {};

        void HandleRPC_EndMatch(
            AzNetworking::IConnection* invokingConnection, const MatchResultsSummary& results) override;

    protected:

    };

    class NetworkMatchComponentController
        : public NetworkMatchComponentControllerBase
    {
    public:
        explicit NetworkMatchComponentController(NetworkMatchComponent& parent);

        void OnActivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
        void OnDeactivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;

        void EndMatch();
        void EndRound();

    private:
        void RoundTickOnceASecond();
        AZ::ScheduledEvent m_roundTickEvent{[this]()
        {
            RoundTickOnceASecond();
        }, AZ::Name("NetworkMatchComponentController")};
    };
}
