/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 * 
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include "MultiplayerSampleSystemComponent.h"

#include <AzCore/Console/ILogger.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/EditContextConstants.inl>

#include <Source/AutoGen/AutoComponentTypes.h>
#include <Source/Weapons/WeaponTypes.h>
#include <Source/Components/NetworkStressTestComponent.h>
#include <Source/Components/NetworkAiComponent.h>
#include <Source/Spawners/RoundRobinSpawner.h>

#include <Multiplayer/IMultiplayer.h>
#include <Multiplayer/Components/NetBindComponent.h>
#include <Multiplayer/ConnectionData/IConnectionData.h>
#include <Multiplayer/ReplicationWindows/IReplicationWindow.h>
#include "AzCore/Component/TransformBus.h"

namespace MultiplayerSample
{
    using namespace AzNetworking;

    void MultiplayerSampleSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        ReflectWeaponEnums(context);
        ClientEffect::Reflect(context);
        GatherParams::Reflect(context);
        HitEffect::Reflect(context);
        WeaponParams::Reflect(context);

        if (AZ::SerializeContext* serialize = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serialize->Class<MultiplayerSampleSystemComponent, AZ::Component>()
                ->Version(0)
                ;

            if (AZ::EditContext* ec = serialize->GetEditContext())
            {
                ec->Class<MultiplayerSampleSystemComponent>("MultiplayerSample", "[Description of functionality provided by this System Component]")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("System"))
                        ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ;
            }
        }
    }

    void MultiplayerSampleSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("MultiplayerSampleService"));
    }

    void MultiplayerSampleSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("MultiplayerSampleService"));
    }

    void MultiplayerSampleSystemComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        required.push_back(AZ_CRC_CE("NetworkingService"));
        required.push_back(AZ_CRC_CE("MultiplayerService"));
    }

    void MultiplayerSampleSystemComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        AZ_UNUSED(dependent);
    }

    void MultiplayerSampleSystemComponent::Init()
    {
        ;
    }

    void MultiplayerSampleSystemComponent::Activate()
    {
        AZ::TickBus::Handler::BusConnect();

        //! Register our gems multiplayer components to assign NetComponentIds
        RegisterMultiplayerComponents();

        AZ::Interface<Multiplayer::IMultiplayerSpawner>::Register(this);
        m_playerSpawner = AZStd::make_unique<RoundRobinSpawner>();
        AZ::Interface<IPlayerSpawner>::Register(m_playerSpawner.get());
    }

    void MultiplayerSampleSystemComponent::Deactivate()
    {
        AZ::Interface<IPlayerSpawner>::Unregister(m_playerSpawner.get());
        AZ::Interface<Multiplayer::IMultiplayerSpawner>::Unregister(this);
        AZ::TickBus::Handler::BusDisconnect();
    }

    void MultiplayerSampleSystemComponent::OnTick([[maybe_unused]] float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time)
    {
        ;
    }

    int MultiplayerSampleSystemComponent::GetTickOrder()
    {
        // Tick immediately after the multiplayer system component
        return AZ::TICK_PLACEMENT + 2;
    }

    void MultiplayerSampleSystemComponent::OnPlayerJoin(
        [[maybe_unused]] uint64_t userId, [[maybe_unused]] const Multiplayer::MultiplayerAgentDatum& agentDatum, AzFramework::EntitySpawnCallback playerSpawnedCallback)
    {
        AZStd::pair<AZ::Data::Asset<AzFramework::Spawnable>, AZ::Transform> entityParams = AZ::Interface<IPlayerSpawner>::Get()->GetNextPlayerSpawn();

        // Callback to move the player to the desired position before it's spawned into the world
        auto preSpawnCallback = [entityParams](
            AzFramework::EntitySpawnTicket::Id /*ticketId*/,
            AzFramework::SpawnableEntityContainerView entities
            )
        {
            const AZ::Entity* e = *entities.begin();
            if (AZ::TransformInterface* transform = e->GetTransform())
            {
                transform->SetWorldTM(entityParams.second);
            }
        };

        auto ticket = AZStd::make_unique<AzFramework::EntitySpawnTicket>(entityParams.first);
        if (ticket->IsValid())
        {
            AzFramework::SpawnAllEntitiesOptionalArgs optionalArgs;
            optionalArgs.m_preInsertionCallback = AZStd::move(preSpawnCallback);
            optionalArgs.m_completionCallback = AZStd::move(playerSpawnedCallback);
            AzFramework::SpawnableEntitiesInterface::Get()->SpawnAllEntities(*ticket, AZStd::move(optionalArgs));
            m_playerSpawnTickets.push_back(AZStd::move(ticket));
        }
    }

    void MultiplayerSampleSystemComponent::OnPlayerLeave(
        Multiplayer::ConstNetworkEntityHandle entityHandle, [[maybe_unused]] const Multiplayer::ReplicationSet& replicationSet, [[maybe_unused]] AzNetworking::DisconnectReason reason)
    {
        AZ::Interface<Multiplayer::IMultiplayer>::Get()->GetNetworkEntityManager()->MarkForRemoval(entityHandle);
    }
}

