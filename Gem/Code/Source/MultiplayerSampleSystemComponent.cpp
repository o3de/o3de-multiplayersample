/*
 * All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
 * its licensors.
 *
 * For complete copyright and license terms please see the LICENSE at the root of this
 * distribution (the "License"). All use of this software is governed by the License,
 * or, if provided, by the license below or the license accompanying this file. Do not
 * remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *
 */

#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/EditContextConstants.inl>

#include <AzNetworking/Framework/INetworking.h>

#include "MultiplayerSampleSystemComponent.h"
#include <Source/AutoGen/AutoComponentTypes.h>

namespace MultiplayerSample
{
    using namespace AzNetworking;

    static const AZStd::string_view s_networkInterfaceName("MultiplayerSampleInterface");

    void MultiplayerSampleSystemComponent::Reflect(AZ::ReflectContext* context)
    {
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
    }

    void MultiplayerSampleSystemComponent::Deactivate()
    {
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

    bool MultiplayerSampleSystemComponent::HandleRequest([[maybe_unused]] IConnection* connection,
        [[maybe_unused]] const IPacketHeader& packetHeader, [[maybe_unused]] const MultiplayerSamplePackets::Sample& packet)
    {
        return true;
    }

    ConnectResult MultiplayerSampleSystemComponent::ValidateConnect([[maybe_unused]] const IpAddress& remoteAddress,
        [[maybe_unused]] const IPacketHeader& packetHeader, [[maybe_unused]] ISerializer& serializer)
    {
        return ConnectResult::Accepted;
    }

    void MultiplayerSampleSystemComponent::OnConnect([[maybe_unused]] IConnection* connection)
    {
        ;
    }

    bool MultiplayerSampleSystemComponent::OnPacketReceived([[maybe_unused]] IConnection* connection, [[maybe_unused]] const IPacketHeader& packetHeader, [[maybe_unused]] ISerializer& serializer)
    {
        return true;
    }

    void MultiplayerSampleSystemComponent::OnPacketLost([[maybe_unused]] IConnection* connection, [[maybe_unused]] PacketId packetId)
    {
        ;
    }

    void MultiplayerSampleSystemComponent::OnDisconnect(IConnection* connection, [[maybe_unused]] DisconnectReason reason, [[maybe_unused]] AzNetworking::TerminationEndpoint endpoint)
    {
        AZLOG_INFO("Disconnected from remote address: %s", connection->GetRemoteAddress().GetString().c_str());
    }
}
