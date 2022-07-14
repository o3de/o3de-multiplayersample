
#include <Source/Components/CannonComponent.h>

#include <AzCore/Component/Entity.h>
#include <AzCore/Component/TransformBus.h>
#include <AzCore/Interface/Interface.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/RTTI/BehaviorContext.h>
#include <AzFramework/Physics/Collision/CollisionEvents.h>
#include <AzFramework/Physics/Common/PhysicsTypes.h>
#include <AzFramework/Physics/PhysicsSystem.h>
#include <AzFramework/Physics/RigidBodyBus.h>
#include <AzCore/Console/ILogger.h>
#include <PhysX/ColliderComponentBus.h>

namespace MultiplayerSample
{
    CannonComponent::CannonComponent()
        : m_onTriggerEnter([this](
            AzPhysics::SimulatedBodyHandle bodyHandle, const AzPhysics::TriggerEvent& triggerEvent)
            {
                this->OnTriggerEnter(bodyHandle, triggerEvent);
            })
    {
    }

    void CannonComponent::Activate()
    {
        // Make sure our collider is a trigger; we don't actually want any true physical response when then cannon ball hits the player.
        // Once the cannon ball hits, we'll destroy it and apply our own cartoon physics to the player.
        AzPhysics::ShapeColliderPairList shapeConfigurations;
        PhysX::ColliderComponentRequestBus::EventResult(shapeConfigurations, GetEntityId(), &PhysX::ColliderComponentRequestBus::Events::GetShapeConfigurations);
        for(auto& [colliderConfig, shapeConfig] : shapeConfigurations)
        {
            if (!colliderConfig->m_isTrigger)
            {
                AZLOG_ERROR("CannonComponent failed! Please change your collider to a trigger.");
                return;
            }
        }

        AZ::TickBus::Handler::BusConnect();
        Physics::RigidBodyRequestBus::Event(GetEntityId(), &Physics::RigidBodyRequestBus::Events::SetKinematic, true);

        if (auto* physicsSystem = AZ::Interface<AzPhysics::SystemInterface>::Get())
        {
            auto [sceneHandle, bodyHandle] = physicsSystem->FindAttachedBodyHandleFromEntityId(GetEntityId());
            AzPhysics::SimulatedBodyEvents::RegisterOnTriggerEnterHandler(sceneHandle, bodyHandle, m_onTriggerEnter);
        }
    }

    void CannonComponent::Deactivate()
    {
        AZ::TickBus::Handler::BusDisconnect();
    }

    void CannonComponent::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<CannonComponent, AZ::Component>()
                ->Version(1)
                ->Field("Velocity", &CannonComponent::m_velocity)
                ->Field("SelfDestructTime", &CannonComponent::m_selfDestructTime)

                ;

            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                editContext->Class<CannonComponent>("CannonComponent", "[Description of functionality provided by this component]")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ->Attribute(AZ::Edit::Attributes::Category, "ComponentCategory")
                        ->Attribute(AZ::Edit::Attributes::Icon, "Icons/Components/Component_Placeholder.svg")
                        ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC_CE("Game"))
                    ->DataElement(AZ::Edit::UIHandlers::Default, &CannonComponent::m_velocity, "Velocity", "Velocity of the cannon ball. Static value for now.")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &CannonComponent::m_selfDestructTime, "Self Destruct Time", "Seconds before self destruction. Use so the cannon doesn't fly forever.")
                    ;
            }
        }

        if (AZ::BehaviorContext* behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            behaviorContext->Class<CannonComponent>("Cannon Component Group")
                ->Attribute(AZ::Script::Attributes::Category, "MultiplayerSample Gem Group")
                ;
        }
    }

    void CannonComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("CannonComponentService"));
    }

    void CannonComponent::GetIncompatibleServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
    }

    void CannonComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        required.push_back(AZ_CRC_CE("PhysicsRigidBodyService"));
    }

    void CannonComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
    }

    void CannonComponent::OnTick(float deltaTime, [[maybe_unused]]AZ::ScriptTimePoint time)
    {
        m_aliveTime += deltaTime;
        if (m_aliveTime > m_selfDestructTime)
        {
            return;
        }

        AZ::Vector3 deltaMove = deltaTime*m_velocity;
        AZ::Vector3 kinematicTarget = this->GetEntity()->GetTransform()->GetWorldTranslation() + deltaMove;

        Physics::RigidBodyRequestBus::Event(GetEntityId(), 
            &Physics::RigidBodyRequestBus::Events::SetKinematicTarget, AZ::Transform::CreateTranslation(kinematicTarget));
    }

    void CannonComponent::OnTriggerEnter([[maybe_unused]]AzPhysics::SimulatedBodyHandle bodyHandle, [[maybe_unused]]const AzPhysics::TriggerEvent& triggerEvent)
    {
        AZLOG_INFO("Cannon OnTriggerEnter");
    }
} // namespace MultiplayerSample

