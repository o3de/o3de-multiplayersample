/*
 * Copyright (c) Contributors to the Open 3D Engine Project
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AzCore/Math/Transform.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/std/smart_ptr/make_shared.h>
#include <AzFramework/Spawnable/SpawnableEntitiesInterface.h>
#include <Components/AttachPlayerWeaponComponent.h>
#include <LmbrCentral/Animation/AttachmentComponentBus.h>

namespace MultiplayerSample
{
    void AttachPlayerWeaponComponent::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<AttachPlayerWeaponComponent, AZ::Component>()
                ->Field( "GunAsset", &AttachPlayerWeaponComponent::m_gunAsset)
                ->Field( "Bone To Attach To", &AttachPlayerWeaponComponent::m_boneToAttachTo)
                ->Field( "Transform", &AttachPlayerWeaponComponent::m_attachmentTransform)
                ->Version(1);

            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                using namespace AZ::Edit;
                editContext->Class<AttachPlayerWeaponComponent>("AttachPlayerWeaponComponent", 
                        "Spawns a non-network gun prefab and attaches it to the player entity.")
                    ->ClassElement(ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Category, "MultiplayerSample")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC_CE("Game"))
                    ->DataElement(AZ::Edit::UIHandlers::Default, &AttachPlayerWeaponComponent::m_gunAsset, "GunAsset", 
                        "non-networked gun prefab to spawn")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &AttachPlayerWeaponComponent::m_boneToAttachTo, "Bone To Attach To", 
                        "The bone on the player actor's skeleton to attach the gun to")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &AttachPlayerWeaponComponent::m_attachmentTransform, "Transform", 
                        "Relative transform of the attachment to the bone")
                ;
            }
        }
    }

    void AttachPlayerWeaponComponent::Activate()
    {
        m_gunTicket = AZStd::make_shared<AzFramework::EntitySpawnTicket>(m_gunAsset);
        
        auto onSpawnedCallback = [this]([[maybe_unused]] AzFramework::EntitySpawnTicket::Id ticketId, AzFramework::SpawnableConstEntityContainerView view)
        {
            for (const AZ::Entity* entity : view)
            {
                LmbrCentral::AttachmentComponentRequestBus::Event(entity->GetId(),
                    &LmbrCentral::AttachmentComponentRequestBus::Events::Attach, GetEntityId(), m_boneToAttachTo.c_str(), m_attachmentTransform);
            }
        };

        AZ_Assert(m_gunTicket->IsValid(), "Unable to instantiate gun's spawnable asset");
        if (m_gunTicket->IsValid())
        {
            AzFramework::SpawnAllEntitiesOptionalArgs optionalArgs;
            optionalArgs.m_completionCallback = AZStd::move(onSpawnedCallback);
            AzFramework::SpawnableEntitiesInterface::Get()->SpawnAllEntities(*m_gunTicket, AZStd::move(optionalArgs));
        }
    }

    void AttachPlayerWeaponComponent::Deactivate()
    {
        m_gunTicket.reset();
    }
}
