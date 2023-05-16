/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of
 * this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include "NetworkTeleportComponent.h"

#include <GameplayEffectsNotificationBus.h>
#include <Multiplayer/Components/NetworkTransformComponent.h>
#include <Source/Components/NetworkTeleportCompatibleComponent.h>
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
            serializationContext->Class<NetworkTeleportComponent, NetworkTeleportComponentBase>()
                ->Version(4);
            NetworkTeleportComponentBase::Reflect(reflection);
        }
    }

    void NetworkTeleportComponent::OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
#if AZ_TRAIT_CLIENT
        m_effect = GetTeleportEffect();
        m_effect.Initialize();
#endif
    }

    void NetworkTeleportComponent::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
#if AZ_TRAIT_CLIENT
        // Clean up the teleport effect emitter.
        m_effect = {};
#endif
    }

#if AZ_TRAIT_CLIENT
    void NetworkTeleportComponent::HandleNotifyTeleport([[maybe_unused]] AzNetworking::IConnection* invokingConnection)
    {
        m_effect.TriggerEffect(GetEntity()->GetTransform()->GetWorldTM());
    }
#endif

    // Controller

    NetworkTeleportComponentController::NetworkTeleportComponentController(NetworkTeleportComponent& parent)
        : NetworkTeleportComponentControllerBase(parent)
        , m_enterTrigger([this](
            AzPhysics::SimulatedBodyHandle bodyHandle,
            const AzPhysics::TriggerEvent& triggerEvent)
            {
                this->OnTriggerEnter(bodyHandle, triggerEvent);
            })
    {
    }

    void NetworkTeleportComponentController::OnPhysicsEnabled(const AZ::EntityId& entityId)
    {
        auto* physicsSystem = AZ::Interface<AzPhysics::SystemInterface>::Get();
        if (physicsSystem)
        {
            auto [sceneHandle, bodyHandle] = physicsSystem->FindAttachedBodyHandleFromEntityId(entityId);
            AzPhysics::SimulatedBodyEvents::RegisterOnTriggerEnterHandler(
                sceneHandle, bodyHandle, m_enterTrigger);
        }
    }

    void NetworkTeleportComponentController::OnPhysicsDisabled([[maybe_unused]] const AZ::EntityId& entityId)
    {
        m_enterTrigger.Disconnect();
    }


    void NetworkTeleportComponentController::OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        Physics::RigidBodyNotificationBus::Handler::BusConnect(GetEntity()->GetId());
    }

    void NetworkTeleportComponentController::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        Physics::RigidBodyNotificationBus::Handler::BusDisconnect();
        m_enterTrigger.Disconnect();
    }

    void NetworkTeleportComponentController::OnTriggerEnter(
        [[maybe_unused]] AzPhysics::SimulatedBodyHandle bodyHandle, const AzPhysics::TriggerEvent& triggerEvent)
    {
        if (triggerEvent.m_otherBody)
        {
            AZ::Vector3 teleportLocation = GetDestinationVector();
            AZ_TracePrintf("Teleporter", "Destination point X, Y is: %f, %f\n",
                teleportLocation.GetX(), teleportLocation.GetY());

            AZ::Entity* otherEntity = GetCollidingEntity(triggerEvent.m_otherBody);

            TeleportPlayer(teleportLocation, otherEntity);
        }
    }

    AZ::Entity* NetworkTeleportComponentController::GetCollidingEntity(
        AzPhysics::SimulatedBody* collidingBody) const
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

    AZ::Vector3 NetworkTeleportComponentController::GetDestinationVector() const
    {
        AZ::Vector3 location = GetEntity()->GetTransform()->
            GetWorldTM().GetTranslation();

        AZ::EntityId destination = GetDestination();

        AZ::TransformBus::EventResult(location, destination,
            &AZ::TransformBus::Events::GetWorldTranslation);

        return location;
    }

    void NetworkTeleportComponentController::TeleportPlayer(
        [[maybe_unused]] const AZ::Vector3& vector, AZ::Entity* entity)
    {
        if (entity)
        {
            MultiplayerSample::NetworkTeleportCompatibleComponent* teleportable =
                entity->FindComponent<MultiplayerSample::NetworkTeleportCompatibleComponent>();
            if (teleportable)
            {
#if AZ_TRAIT_SERVER
                NotifyTeleport();
                teleportable->Teleport(vector);
#endif
            }
            else
            {
                AZ_TracePrintf("Teleporter", "colliding entity %s is not teleport compatible! NoOp.\n",
                    entity->GetName().c_str());
            }
        }
    }

} // namespace MultiplayerSample
