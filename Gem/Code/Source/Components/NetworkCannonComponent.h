
#pragma once
#include <AzCore/Asset/AssetCommon.h>
#include <AzCore/Component/Component.h>
#include <AzCore/Component/TickBus.h>
#include <AzCore/Math/Vector3.h>
#include <AzFramework/Physics/Common/PhysicsSimulatedBodyEvents.h>
#include <AzFramework/Spawnable/SpawnableEntitiesInterface.h>
#include <Source/AutoGen/NetworkCannonComponent.AutoComponent.h>


namespace MultiplayerSample
{
    class NetworkCannonComponent
        : public NetworkCannonComponentBase
    {
    public:
        AZ_MULTIPLAYER_COMPONENT(MultiplayerSample::NetworkCannonComponent, s_networkCannonComponentConcreteUuid, MultiplayerSample::NetworkCannonComponentBase);

        static void Reflect(AZ::ReflectContext* context);
        
        void OnInit() override;
        void OnActivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
        void OnDeactivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
        void HandleOnHit(AzNetworking::IConnection* invokingConnection) override;

        /*
        * Specifies the services that this component provides.
        * Other components can declare a dependency on these services. The system uses this
        * information to determine the order of component activation.
        */
        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);

        /*
        * Specifies the services that this component requires.
        * The system activates the required services before it activates this component.
        * It also deactivates the required services after it deactivates this component.
        * If a required service is missing before this component is activated, the system
        * returns an error and does not activate this component.
        */
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
    
    private:
        AzFramework::EntitySpawnTicket m_onHitParticleFxTicket;
    };


    class NetworkCannonComponentController
        : public NetworkCannonComponentControllerBase
        , AZ::TickBus::Handler
    {
    public:
        NetworkCannonComponentController(NetworkCannonComponent& parent);

        // NetworkCannonComponentControllerBase override...
        void OnActivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
        void OnDeactivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
      
    private:
        // AZ::TickBus::Handler override...
        void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;

        void OnTriggerEnter(AzPhysics::SimulatedBodyHandle bodyHandle, const AzPhysics::TriggerEvent& triggerEvent);

        float m_aliveTime = 0.0f;
        AzPhysics::SimulatedBodyEvents::OnTriggerEnter::Handler m_onTriggerEnter;
    };
} // namespace MultiplayerSample
