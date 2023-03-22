/*
 * Copyright (c) Contributors to the Open 3D Engine Project
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AzCore/Serialization/EditContext.h>
#include <AzCore/std/smart_ptr/make_shared.h>
#include <AzFramework/Spawnable/SpawnableEntitiesInterface.h>
#include <Components/AttachPlayerWeaponComponent.h>
#include <LmbrCentral/Animation/AttachmentComponentBus.h>

namespace MultiplayerSample
{
    void AttachPlayerWeaponComponent::Reflect(AZ::ReflectContext* context)
    {
        AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context);
        if (serializeContext)
        {
            serializeContext->Class<AttachPlayerWeaponComponent, AZ::Component>()
                ->Field( "GunAsset", &AttachPlayerWeaponComponent::m_gunSpawnableAsset)
                ->Version(1);

            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                using namespace AZ::Edit;
                editContext->Class<AttachPlayerWeaponComponent>("AttachPlayerWeaponComponent", 
                        "Spawns a non-network gun prefab and attaches it to the player entity.")
                    ->ClassElement(ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Category, "MultiplayerSample")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC_CE("Game"))
                    ->DataElement(nullptr, &AttachPlayerWeaponComponent::m_gunSpawnableAsset, "GunAsset", "non-networked gun prefab to spawn")
                ;
            }
        }
    }

    void AttachPlayerWeaponComponent::Activate()
    {
        auto ticket = AZStd::make_shared<AzFramework::EntitySpawnTicket>(m_gunSpawnableAsset);
        
        auto onSpawnedCallback = [ticket]([[maybe_unused]] AzFramework::EntitySpawnTicket::Id ticketId, AzFramework::SpawnableConstEntityContainerView view)
        {
            for (const AZ::Entity* entity : view)
            {
                LmbrCentral::AttachmentComponentRequestBus::Event(entity->GetId(),
                    &LmbrCentral::AttachmentComponentRequestBus::Events::);
            }


            const AZ::Entity* rootEntity = *view.begin();
            /*if (AzFramework::TransformComponent* entityTransform = rootEntity->FindComponent<AzFramework::TransformComponent>())
            {
                entityTransform->SetWorldTM(world);
            }*/
        };

        AZ_Assert(ticket->IsValid(), "Unable to instantiate spawnable asset");
        if (ticket->IsValid())
        {
            AzFramework::SpawnAllEntitiesOptionalArgs optionalArgs;
            optionalArgs.m_completionCallback = AZStd::move(onSpawnedCallback);
            AzFramework::SpawnableEntitiesInterface::Get()->SpawnAllEntities(*ticket, AZStd::move(optionalArgs));
        }
    }

    void AttachPlayerWeaponComponent::Deactivate()
    {
    }
}
