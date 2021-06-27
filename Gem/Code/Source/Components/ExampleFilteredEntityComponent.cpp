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

#include <AzCore/Serialization/EditContext.h>
#include <Components/ExampleFilteredEntityComponent.h>

namespace MultiplayerSample
{
    void ExampleFilteredEntityComponent::Reflect(AZ::ReflectContext* context)
    {
        AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context);
        if (serializeContext)
        {
            serializeContext->Class<ExampleFilteredEntityComponent, AZ::Component>()
                ->Field("Hide from Odd Connections", &ExampleFilteredEntityComponent::m_filteredEntitiesForOddConnections)
                ->Field("Hide from Even Connections", &ExampleFilteredEntityComponent::m_filteredEntitiesForEvenConnections)
                ->Version(1);

            if (AZ::EditContext* ptrEdit = serializeContext->GetEditContext())
            {
                using namespace AZ::Edit;
                ptrEdit->Class<ExampleFilteredEntityComponent>("ExampleFilteredEntityComponent", "An example of filtering entities out in network replication")
                    ->ClassElement(ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Category, "MultiplayerSample")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC_CE("Game"))
                    ->DataElement(UIHandlers::Default, &ExampleFilteredEntityComponent::m_filteredEntitiesForOddConnections, "Hide from Odd Connections", "")
                    ->DataElement(UIHandlers::Default, &ExampleFilteredEntityComponent::m_filteredEntitiesForEvenConnections, "Hide from Even Connections", "");
            }
        }
    }

    void ExampleFilteredEntityComponent::Activate()
    {
        if (Multiplayer::GetMultiplayer()->GetAgentType() != Multiplayer::MultiplayerAgentType::Client)
        {
            Multiplayer::FilteredServerToClientRequestBus::Broadcast(&Multiplayer::FilteredServerToClientRequestBus::Events::SetFilteredInterface, this);
            Multiplayer::FilteredServerToClientNotificationBus::Handler::BusConnect();
        }
    }

    void ExampleFilteredEntityComponent::Deactivate()
    {
        if (Multiplayer::GetMultiplayer()->GetAgentType() != Multiplayer::MultiplayerAgentType::Client)
        {
            Multiplayer::FilteredServerToClientNotificationBus::Handler::BusDisconnect();
            Multiplayer::FilteredServerToClientRequestBus::Broadcast(&Multiplayer::FilteredServerToClientRequestBus::Events::SetFilteredInterface, nullptr);
        }
    }

    bool ExampleFilteredEntityComponent::IsEntityFiltered(
        [[maybe_unused]] AZ::Entity* entity,
        [[maybe_unused]] Multiplayer::ConstNetworkEntityHandle controllerEntity,
        [[maybe_unused]] const AzNetworking::ConnectionId connectionId)
    {
        // Note: @IsEntityFiltered can be a very hot path, so do your best to optimize this method.
        // This example component just uses entity names for filtering, for the sake of an example.
        // In practice, one might implement the lookup using a specialized structure such as AZStd::map<AZ::EntityId, bool>, etc.

        const bool evenConnectionId = static_cast<uint32_t>(connectionId) % 2 == 0;

        if (entity->GetName().starts_with( evenConnectionId ? "Even" : "Odd" ))
        {
            return true;
        }

        return false;
    }

    void ExampleFilteredEntityComponent::OnFilteredServerToClientActivated(AZ::EntityId controllerEntity)
    {
        Multiplayer::FilteredServerToClientRequestBus::Event(controllerEntity, &Multiplayer::FilteredServerToClientRequestBus::Events::SetFilteredInterface, this);
    }
}
