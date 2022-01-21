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
#include <AzFramework/Session/ISessionHandlingRequests.h>

#include <Source/AutoGen/AutoComponentTypes.h>
#include <Source/Weapons/WeaponTypes.h>
#include <Source/Components/NetworkStressTestComponent.h>
#include <Source/Components/NetworkAiComponent.h>

#include <Multiplayer/IMultiplayer.h>
#include <Multiplayer/Components/NetBindComponent.h>
#include <Multiplayer/ConnectionData/IConnectionData.h>
#include <Multiplayer/ReplicationWindows/IReplicationWindow.h>

namespace MultiplayerSample
{
    using namespace AzNetworking;

    AZ_CVAR(AZ::CVarFixedString, sv_playerSpawnAsset, "prefabs/player.network.spawnable", nullptr, AZ::ConsoleFunctorFlags::DontReplicate,
        "The spawnable to use when a new player connects");

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
    }

    void MultiplayerSampleSystemComponent::Deactivate()
    {
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

    AZStd::pair<Multiplayer::PrefabEntityId, AZ::Transform> MultiplayerSampleSystemComponent::OnPlayerJoin(
        uint64_t userId,
        [[maybe_unused]] AzFramework::PlayerConnectionConfig config,
        [[maybe_unused]] Multiplayer::LongNetworkString ticket)
    {
        auto sv_playerSpawnAssetLowerCase = static_cast<AZ::CVarFixedString>(sv_playerSpawnAsset);
        AZStd::to_lower(sv_playerSpawnAssetLowerCase.begin(), sv_playerSpawnAssetLowerCase.end());
        Multiplayer::PrefabEntityId playerPrefabEntityId(AZ::Name(sv_playerSpawnAssetLowerCase.c_str()));

        // Assuming userIds increase linearly (which is naive), spawn in rows of a prescribed size
        const uint8_t spawnRowSize = 8;
        AZ::Transform transform = AZ::Transform::CreateIdentity();
        transform.SetTranslation(
            AZ::Vector3(aznumeric_cast<float>(userId % spawnRowSize) * 32.f, aznumeric_cast<float>(userId / spawnRowSize) * 32.f, 0));

        return AZStd::pair<Multiplayer::PrefabEntityId, AZ::Transform>(playerPrefabEntityId, transform);
    }

    void MultiplayerSampleSystemComponent::OnPlayerLeave(
        Multiplayer::ConstNetworkEntityHandle entityHandle, [[maybe_unused]] const Multiplayer::ReplicationSet& replicationSet, [[maybe_unused]] AzNetworking::DisconnectReason reason)
    {
        AZ::Interface<Multiplayer::IMultiplayer>::Get()->GetNetworkEntityManager()->MarkForRemoval(entityHandle);
    }
}

