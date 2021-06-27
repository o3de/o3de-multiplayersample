/*
 * All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
 * its licensors.
 *
 * For complete copyright and license terms please see the LICENSE at the root of this
 * distribution (the "License"). All use of this software is governed by the License,
 * or, if provided, by the license below or the license accompanying this file. Do not
 * remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
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
