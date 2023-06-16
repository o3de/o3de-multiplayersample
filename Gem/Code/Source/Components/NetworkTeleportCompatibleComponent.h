/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of
 * this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <Source/AutoGen/NetworkTeleportCompatibleComponent.AutoComponent.h>
#include <Source/Effects/GameEffect.h>

namespace MultiplayerSample
{
    class NetworkTeleportCompatibleComponent
        : public NetworkTeleportCompatibleComponentBase
    {
    public:
        AZ_MULTIPLAYER_COMPONENT(
            MultiplayerSample::NetworkTeleportCompatibleComponent, s_networkTeleportCompatibleComponentConcreteUuid, MultiplayerSample::NetworkTeleportCompatibleComponentBase);

        static void Reflect(AZ::ReflectContext* context);

        void OnInit() override {}
        void OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating) override;
        void OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating) override;

#if AZ_TRAIT_CLIENT
        void HandleNotifyTeleport(AzNetworking::IConnection* invokingConnection, const AZ::Vector3& teleportedLocation) override;
#endif

    private:
        GameEffect m_effect;
    };

    class NetworkTeleportCompatibleComponentController
        : public NetworkTeleportCompatibleComponentControllerBase
    {
    public:
        NetworkTeleportCompatibleComponentController(NetworkTeleportCompatibleComponent& parent);

        void OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating) override {};
        void OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating) override {};

#if AZ_TRAIT_SERVER
        void HandleTeleport(
            AzNetworking::IConnection* invokingConnection, const AZ::Vector3& teleportedLocation) override;
#endif
    };
} // namespace MultiplayerSample
