/*
 * Copyright (c) Contributors to the Open 3D Engine Project
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include "NetworkTeleportComponent.h"
#include <Multiplayer/Components/NetworkTransformComponent.h>

#include <AzCore/Component/TransformBus.h>
#include <AzCore/Component/ComponentApplicationBus.h>
#include <AzCore/Component/Entity.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Interface/Interface.h>
#include <AzFramework/Physics/RigidBodyBus.h>
#include <AzFramework/Physics/PhysicsSystem.h>

namespace MultiplayerSample
{
    void NetworkTeleportComponent::Reflect(AZ::ReflectContext* reflection)
    {
        if (auto serializationContext = azrtti_cast<AZ::SerializeContext*>(reflection))
        {
            serializationContext->Class<NetworkTeleportComponent, Component>()
                ->Field("Destination", &NetworkTeleportComponent::m_reset)
                ->Version(3);

            if (AZ::EditContext* editContext = serializationContext->GetEditContext())
            {
                editContext->Class<NetworkTeleportComponent>("Network Teleport Component",
                    "Teleports colliding player entities to a fixed location")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Category, 
                        "MultiplayerSample")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, 
                        AZ_CRC_CE("Game"))
                    ->DataElement(0, &NetworkTeleportComponent::m_reset,
                        "Destination", "Entity at which to teleport the colliding player")
                    ;
            }
        }
    }

    NetworkTeleportComponent::NetworkTeleportComponent()
        : m_enterTrigger([this](
            AzPhysics::SimulatedBodyHandle bodyHandle, 
            const AzPhysics::TriggerEvent& triggerEvent)
            {
                this->OnTriggerEnter(bodyHandle, triggerEvent);
            })
    {
    }

    void NetworkTeleportComponent::Activate()
    {
        auto* physicsSystem = AZ::Interface<AzPhysics::SystemInterface>::Get();
         if (physicsSystem)
        {
            auto [sceneHandle, bodyHandle] = physicsSystem->FindAttachedBodyHandleFromEntityId(GetEntityId());
            AzPhysics::SimulatedBodyEvents::RegisterOnTriggerEnterHandler(
                sceneHandle, bodyHandle, m_enterTrigger);
        }
    }

    void NetworkTeleportComponent::OnTriggerEnter(
        [[maybe_unused]] AzPhysics::SimulatedBodyHandle bodyHandle, const AzPhysics::TriggerEvent& triggerEvent)
    {

        if (triggerEvent.m_otherBody)
        {
            // TODO: validate it's a player entity
            AZ::Vector3 teleportLocation = GetDestination();
            AZ_TracePrintf("Teleporter", "destination point X is: %f\n",
                teleportLocation.GetX());

            AZ::Entity* otherEntity = GetCollidingEntity(triggerEvent.m_otherBody);
            TeleportPlayer(teleportLocation, otherEntity);
        }
    }

    AZ::Entity* NetworkTeleportComponent::GetCollidingEntity(AzPhysics::SimulatedBody* collidingBody) const
    {
        AZ::Entity* collidingEntity = nullptr;		
        if (collidingBody)
        {
            AZ::EntityId collidingEntityId = collidingBody->GetEntityId();
            AZ::ComponentApplicationBus::BroadcastResult(
                collidingEntity, &AZ::ComponentApplicationBus::Events::FindEntity, collidingEntityId);
        }
        return collidingEntity;
    }

    AZ::Vector3 NetworkTeleportComponent::GetDestination() const
    {
        AZ::Vector3 newLocation = AZ::Vector3::CreateZero();
        AZ::TransformBus::EventResult(newLocation, m_reset,
            &AZ::TransformBus::Events::GetWorldTranslation);

        return newLocation;
    }

    void NetworkTeleportComponent::TeleportPlayer(
        const AZ::Vector3& v, AZ::Entity* entity)
    {
        if (entity)
        {
            AZ::EntityId playerToTeleport = entity->GetId();

            AZ_Printf(
                "Teleporter", "Attempting to teleport entity: %s\n", entity->GetName().c_str());

            // disable physics so we can move entity
            Physics::RigidBodyRequestBus::Event(playerToTeleport,
                &Physics::RigidBodyRequestBus::Events::DisablePhysics);

            // move the entity
            AZ::TransformBus::Event(playerToTeleport,
                &AZ::TransformBus::Events::SetWorldTranslation, v);

            // increment resetCount so it doesn't try to interplate from last location
            Multiplayer::NetworkTransformComponent* netTransform =
                entity->FindComponent<Multiplayer::NetworkTransformComponent>();

            if (netTransform) {
                auto controller = 
                    static_cast<Multiplayer::NetworkTransformComponentController*>(netTransform->GetController());

                AZ_TracePrintf(
                    "Teleporter", "updating resetCount to : %u\n", (controller->GetResetCount() + 1));

                controller->SetResetCount(controller->GetResetCount() + 1);
            }

            // re-enable physics
            Physics::RigidBodyRequestBus::Event(playerToTeleport,
                &Physics::RigidBodyRequestBus::Events::EnablePhysics);
        }
    }
}
