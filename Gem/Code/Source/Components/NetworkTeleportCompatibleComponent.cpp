#include <Source/Components/NetworkTeleportCompatibleComponent.h>

#include <Multiplayer/Components/NetworkTransformComponent.h>
#include <AzCore/Component/TransformBus.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzFramework/Physics/RigidBodyBus.h>

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

    // Controller

    NetworkTeleportCompatibleComponentController::NetworkTeleportCompatibleComponentController(NetworkTeleportCompatibleComponent& parent)
        : NetworkTeleportCompatibleComponentControllerBase(parent)
    {
    }

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
        Physics::RigidBodyRequestBus::Event(selfId, &Physics::RigidBodyRequestBus::Events::DisablePhysics);

        // move self and increment resetCount to prevent transform interpolation
        AZ::TransformBus::Event(selfId,
            &AZ::TransformBus::Events::SetWorldTranslation, teleportedLocation);

        Multiplayer::NetworkTransformComponentController* netTransform = GetNetworkTransformComponentController();
        netTransform->SetResetCount(netTransform->GetResetCount() + 1);

        // re-enable physics
        Physics::RigidBodyRequestBus::Event(selfId, &Physics::RigidBodyRequestBus::Events::EnablePhysics);
    }

} // namespace MultiplayerSample
