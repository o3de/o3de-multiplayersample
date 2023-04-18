/*
 * Copyright (c) Contributors to the Open 3D Engine Project
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AzCore/Asset/AssetSerializer.h>
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
                ->Field( "WeaponAsset", &AttachPlayerWeaponComponent::m_weaponAsset)
                ->Field( "Bone To Attach To", &AttachPlayerWeaponComponent::m_boneToAttachTo)
                ->Field( "Transform", &AttachPlayerWeaponComponent::m_attachmentTransform)
                ->Version(1);

            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                using namespace AZ::Edit;
                editContext->Class<AttachPlayerWeaponComponent>("AttachPlayerWeaponComponent", 
                        "Spawns a non-network weapon prefab and attaches it to the player entity.")
                    ->ClassElement(ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Category, "MultiplayerSample")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC_CE("Game"))
                    ->DataElement(AZ::Edit::UIHandlers::Default, &AttachPlayerWeaponComponent::m_weaponAsset, "WeaponAsset", 
                        "non-networked weapon prefab to spawn")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &AttachPlayerWeaponComponent::m_boneToAttachTo, "Bone To Attach To", 
                        "The bone on the player actor's skeleton to attach the weapon to")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &AttachPlayerWeaponComponent::m_attachmentTransform, "Transform", 
                        "Relative transform of the attachment to the bone")
                ;
            }
        }
    }

    void AttachPlayerWeaponComponent::Activate()
    {
        m_weaponTicket = AZStd::make_shared<AzFramework::EntitySpawnTicket>(m_weaponAsset);
        
        auto onSpawnedCallback = [this]([[maybe_unused]] AzFramework::EntitySpawnTicket::Id ticketId, AzFramework::SpawnableConstEntityContainerView view)
        {
            for (const AZ::Entity* entity : view)
            {
                LmbrCentral::AttachmentComponentRequestBus::Event(entity->GetId(),
                    &LmbrCentral::AttachmentComponentRequestBus::Events::Attach, GetEntityId(), m_boneToAttachTo.c_str(), m_attachmentTransform);
            }
        };

        AZ_Assert(m_weaponTicket->IsValid(), "Unable to instantiate weapon's spawnable asset");
        if (m_weaponTicket->IsValid())
        {
            AzFramework::SpawnAllEntitiesOptionalArgs optionalArgs;
            optionalArgs.m_completionCallback = AZStd::move(onSpawnedCallback);
            AzFramework::SpawnableEntitiesInterface::Get()->SpawnAllEntities(*m_weaponTicket, AZStd::move(optionalArgs));
        }
    }

    void AttachPlayerWeaponComponent::Deactivate()
    {
        m_weaponTicket.reset();
    }
}
