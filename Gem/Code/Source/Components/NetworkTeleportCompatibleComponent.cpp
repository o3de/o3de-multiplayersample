/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of
 * this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <Source/Components/NetworkTeleportCompatibleComponent.h>
#include <Multiplayer/Components/NetworkTransformComponent.h>
#include <AzCore/Component/TransformBus.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzFramework/Physics/Components/SimulatedBodyComponentBus.h>

namespace MultiplayerSample
{
    void NetworkTeleportCompatibleComponent::Reflect(AZ::ReflectContext* context)
    {
        AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context);
        if (serializeContext)
        {
            serializeContext->Class<NetworkTeleportCompatibleComponent, NetworkTeleportCompatibleComponentBase>()
                ->Version(1);
        }
        NetworkTeleportCompatibleComponentBase::Reflect(context);
    }

    void NetworkTeleportCompatibleComponent::OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
#if AZ_TRAIT_CLIENT
        m_effect = GetTeleportEffect();
        m_effect.Initialize();
#endif
    }

    void NetworkTeleportCompatibleComponent::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
#if AZ_TRAIT_CLIENT
        // Clean up the teleport effect emitter.
        m_effect = {};
#endif
    }

#if AZ_TRAIT_CLIENT
    void NetworkTeleportCompatibleComponent::HandleNotifyTeleport([[maybe_unused]] AzNetworking::IConnection* invokingConnection, const AZ::Vector3& teleportedLocation)
    {
        const AZ::Transform transform = AZ::Transform::CreateFromQuaternionAndTranslation(AZ::Quaternion::CreateIdentity(), teleportedLocation);
        m_effect.TriggerEffect(transform);
    }
#endif

    // Controller

    NetworkTeleportCompatibleComponentController::NetworkTeleportCompatibleComponentController(NetworkTeleportCompatibleComponent& parent)
        : NetworkTeleportCompatibleComponentControllerBase(parent)
    {
    }

#if AZ_TRAIT_SERVER
    void  NetworkTeleportCompatibleComponentController::HandleTeleport(
        [[maybe_unused]] AzNetworking::IConnection* invokingConnection, [[maybe_unused]] const AZ::Vector3& teleportedLocation)
    {
        AZ::Entity* self = GetEntity();
        
        AZ_TracePrintf("TeleportCompatibleComponent", "Teleporting entity %s to (%f,%f)\n", 
            self->GetName().c_str(),
            teleportedLocation.GetX(), 
            teleportedLocation.GetY());

        AZ::EntityId selfId = self->GetId();
        
        // disable physics (needed to move rigid bodies)
        // see: https://github.com/o3de/o3de/issues/2541
        AzPhysics::SimulatedBodyComponentRequestsBus::Event(selfId, &AzPhysics::SimulatedBodyComponentRequestsBus::Events::DisablePhysics);

        // move self and increment resetCount to prevent transform interpolation
        AZ::TransformBus::Event(selfId,
            &AZ::TransformBus::Events::SetWorldTranslation, teleportedLocation);

        Multiplayer::NetworkTransformComponentController* netTransform = GetNetworkTransformComponentController();
        netTransform->SetResetCount(netTransform->GetResetCount() + 1);

        // re-enable physics
        AzPhysics::SimulatedBodyComponentRequestsBus::Event(selfId, &AzPhysics::SimulatedBodyComponentRequestsBus::Events::EnablePhysics);

        NotifyTeleport(teleportedLocation);
    }
#endif
} // namespace MultiplayerSample
