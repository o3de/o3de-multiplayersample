/*
 * Copyright (c) Contributors to the Open 3D Engine Project
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/Component/Component.h>
#include <Multiplayer/Components/FilteredReplicationInterface.h>
#include <Multiplayer/Components/FilteredServerToClientBus.h>

namespace MultiplayerSample
{
    class ExampleFilteredEntityComponent final
        : public AZ::Component
        , public Multiplayer::FilteredReplicationInterface
        , public Multiplayer::FilteredServerToClientNotificationBus::Handler
    {
    public:
        AZ_COMPONENT(MultiplayerSample::ExampleFilteredEntityComponent, "{7BF3BF1D-383A-40E7-BCF2-1ED5B2D2A43C}");

        static void Reflect(AZ::ReflectContext* context);

        void Activate() override;
        void Deactivate() override;

        //! FilteredReplicationInterface overrides.
        bool IsEntityFiltered(AZ::Entity* entity, Multiplayer::ConstNetworkEntityHandle controllerEntity, const AzNetworking::ConnectionId connectionId) override;

        // Multiplayer::FilteredServerToClientNotificationBus overrides.
        void OnFilteredServerToClientActivated(AZ::EntityId controllerEntity) override;

    private:
        AZStd::vector<AZ::EntityId> m_filteredEntitiesForOddConnections;
        AZStd::vector<AZ::EntityId> m_filteredEntitiesForEvenConnections;
    };
}
