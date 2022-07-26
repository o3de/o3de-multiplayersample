#pragma once

#include <Source/AutoGen/NetworkTeleportCompatibleComponent.AutoComponent.h>

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
        void OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating) override {};
        void OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating) override {};
    };

    class NetworkTeleportCompatibleComponentController
        : public NetworkTeleportCompatibleComponentControllerBase
    {
    public:
        NetworkTeleportCompatibleComponentController(NetworkTeleportCompatibleComponent& parent);

        void OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating) override {};
        void OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating) override {};

        void HandleTeleport(
            AzNetworking::IConnection* invokingConnection, const AZ::Vector3& teleportedLocation) override;
    };

} // namespace MultiplayerSample