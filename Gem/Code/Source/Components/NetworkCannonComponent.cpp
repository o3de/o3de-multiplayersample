
#include <Source/Components/NetworkCannonComponent.h>

#include <AzCore/Component/Entity.h>
#include <AzCore/Component/TransformBus.h>
#include <AzCore/Console/ILogger.h>
#include <AzCore/Interface/Interface.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/RTTI/BehaviorContext.h>
#include <AzFramework/Components/TransformComponent.h>
#include <AzFramework/Physics/Collision/CollisionEvents.h>
#include <AzFramework/Physics/Common/PhysicsTypes.h>
#include <AzFramework/Physics/PhysicsSystem.h>
#include <AzFramework/Physics/RigidBodyBus.h>
#include <PhysX/ColliderComponentBus.h>


namespace MultiplayerSample
{
    // Component...
    void NetworkCannonComponent::OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
    }

    void NetworkCannonComponent::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
    }

    void NetworkCannonComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<NetworkCannonComponent, NetworkCannonComponentBase>()
                ->Version(1);
        }
        NetworkCannonComponentBase::Reflect(context);
    }

    void NetworkCannonComponent::OnInit()
    {
    }

    void NetworkCannonComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        NetworkCannonComponentBase::GetProvidedServices(provided);
        provided.push_back(AZ_CRC_CE("NetworkCannonComponentService"));
    }

    void NetworkCannonComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        NetworkCannonComponentBase::GetRequiredServices(required);
        required.push_back(AZ_CRC_CE("PhysicsRigidBodyService"));
    }

    void NetworkCannonComponent::HandleOnHit([[maybe_unused]]AzNetworking::IConnection* invokingConnection)
    {
        // Authority has alerted us there was a hit. Play explosion particle fx.
        m_onHitParticleFxTicket = AzFramework::EntitySpawnTicket(GetOnHitParticleFx());

        if (m_onHitParticleFxTicket.IsValid())
        {
            AZ::Transform world = GetEntity()->GetTransform()->GetWorldTM();
            auto cb = [world](AzFramework::EntitySpawnTicket::Id /*ticketId*/, AzFramework::SpawnableEntityContainerView view){
                const AZ::Entity* e = *view.begin();
                if (auto transform = e->FindComponent<AzFramework::TransformComponent>())
                {
                    transform->SetWorldTM(world);
                }
            };

            AzFramework::SpawnAllEntitiesOptionalArgs optionalArgs;
            optionalArgs.m_preInsertionCallback = AZStd::move(cb);
            AzFramework::SpawnableEntitiesInterface::Get()->SpawnAllEntities(m_onHitParticleFxTicket, AZStd::move(optionalArgs));
        }
    }

    // Controller...
    NetworkCannonComponentController::NetworkCannonComponentController(NetworkCannonComponent& parent)
        : NetworkCannonComponentControllerBase(parent)
        , m_onTriggerEnter([this](
            AzPhysics::SimulatedBodyHandle bodyHandle, const AzPhysics::TriggerEvent& triggerEvent)
            {
                this->OnTriggerEnter(bodyHandle, triggerEvent);
            })
    {
    }

    void NetworkCannonComponentController::OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        // Make sure our collider is a trigger; we don't actually want any true physical response when then cannon ball hits the player.
        // Once the cannon ball hits, we'll destroy it and apply our own cartoon physics to the player.
        AZ::EntityId entityId = GetEntity()->GetId();
        AzPhysics::ShapeColliderPairList shapeConfigurations;
        PhysX::ColliderComponentRequestBus::EventResult(shapeConfigurations, entityId, &PhysX::ColliderComponentRequestBus::Events::GetShapeConfigurations);
        for(auto& [colliderConfig, shapeConfig] : shapeConfigurations)
        {
            if (!colliderConfig->m_isTrigger)
            {
                AZLOG_ERROR("NetworkCannonComponent failed! Please change your collider to a trigger.");
                return;
            }
        }

        AZ::TickBus::Handler::BusConnect();
        Physics::RigidBodyRequestBus::Event(entityId, &Physics::RigidBodyRequestBus::Events::SetKinematic, true);

        // Listen for objects hitting our cannon ball
        if (auto* physicsSystem = AZ::Interface<AzPhysics::SystemInterface>::Get())
        {
            auto [sceneHandle, bodyHandle] = physicsSystem->FindAttachedBodyHandleFromEntityId(entityId);
            AzPhysics::SimulatedBodyEvents::RegisterOnTriggerEnterHandler(sceneHandle, bodyHandle, m_onTriggerEnter);
        }
    }

    void NetworkCannonComponentController::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        AZ::TickBus::Handler::BusDisconnect();
    }


    void NetworkCannonComponentController::OnTick(float deltaTime, [[maybe_unused]]AZ::ScriptTimePoint time)
    {
        m_aliveTime += deltaTime;
        if (m_aliveTime > GetSelfDestructTime())
        {
            AZ::TickBus::Handler::BusDisconnect();
            return;
        }

        AZ::Vector3 deltaMove = deltaTime*GetVelocity();
        AZ::Vector3 kinematicTarget = this->GetEntity()->GetTransform()->GetWorldTranslation() + deltaMove;

        Physics::RigidBodyRequestBus::Event(GetEntity()->GetId(), 
            &Physics::RigidBodyRequestBus::Events::SetKinematicTarget, AZ::Transform::CreateTranslation(kinematicTarget));
    }

    void NetworkCannonComponentController::OnTriggerEnter([[maybe_unused]]AzPhysics::SimulatedBodyHandle bodyHandle, [[maybe_unused]]const AzPhysics::TriggerEvent& triggerEvent)
    {
        AZLOG_INFO("Cannon OnTriggerEnter");
        OnHit(); //< Send RPC to tell the clients we made a hit!
        AZ::TickBus::Handler::BusDisconnect();
    }
} // namespace MultiplayerSample
