/*
 * Copyright (c) Contributors to the Open 3D Engine Project
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AzCore/Serialization/EditContext.h>
#include <AzFramework/Visibility/OcclusionBus.h>
#include <Components/OcclusionFilteredEntityComponent.h>
#include <Multiplayer/IMultiplayer.h>
#include <Multiplayer/Components/NetBindComponent.h>

namespace MultiplayerSample
{
    void OcclusionFilteredEntityComponent::Reflect(AZ::ReflectContext* context)
    {
        AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context);
        if (serializeContext)
        {
            serializeContext->Class<OcclusionFilteredEntityComponent, AZ::Component>()
                ->Version(1);

            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                editContext->Class<OcclusionFilteredEntityComponent>("OcclusionFilteredEntityComponent", "Filters occluded player entities out of network replication")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Category, "MultiplayerSample")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC_CE("Level"))
                ;
            }
        }
    }

    void OcclusionFilteredEntityComponent::Activate()
    {
        AZ::Interface<IFilterEntityManager>::Register(this);

        // Create an occlusion view just for performing clear to player visibility tests.
        AzFramework::OcclusionRequestBus::Broadcast(&AzFramework::OcclusionRequestBus::Events::CreateOcclusionView, m_occlusionViewName);
    }

    void OcclusionFilteredEntityComponent::Deactivate()
    {
        AZ::Interface<IFilterEntityManager>::Unregister(this);
        AzFramework::OcclusionRequestBus::Broadcast(&AzFramework::OcclusionRequestBus::Events::DestroyOcclusionView, m_occlusionViewName);
    }

    bool OcclusionFilteredEntityComponent::IsEntityFiltered(
        [[maybe_unused]] AZ::Entity* entity,
        [[maybe_unused]] Multiplayer::ConstNetworkEntityHandle controllerEntity,
        [[maybe_unused]] AzNetworking::ConnectionId connectionId)
    {
        bool result = false;

        // If one is available, use the occlusion culling system to filter out players that cannot be seen by the controlled player.
        const auto entityNetBindComponent = entity->FindComponent<Multiplayer::NetBindComponent>();
        const auto entityConnectionId = entityNetBindComponent ? entityNetBindComponent->GetOwningConnectionId() : AzNetworking::InvalidConnectionId;
        if (entityConnectionId != AzNetworking::InvalidConnectionId)
        {
            AzFramework::OcclusionRequestBus::Broadcast([&](AzFramework::OcclusionRequestBus::Events* occlusionHandler) {
                // Check for a preexisting occlusion view or create one if necessary.
                if (!occlusionHandler->IsOcclusionView(m_occlusionViewName))
                {
                    occlusionHandler->CreateOcclusionView(m_occlusionViewName);
                }
                if (occlusionHandler->IsOcclusionView(m_occlusionViewName))
                {
                    // Perform an occlusion query to determine if the controlled entity can see the filtered entity.
                    const AZStd::vector<bool> visibility = occlusionHandler->GetOcclusionViewEntityToEntityVisibility(
                        m_occlusionViewName, controllerEntity.GetEntity()->GetId(), AZStd::vector<AZ::EntityId>{ entity->GetId() }
                    );
                    // If the query succeeded and the entity cannot be seend then filter it out from network replication.
                    result = !visibility.empty() && !visibility[0];
                }
                });
        }
        return result;
    }
}
