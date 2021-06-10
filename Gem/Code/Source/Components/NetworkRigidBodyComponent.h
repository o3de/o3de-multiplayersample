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
#pragma once

#include <Source/AutoGen/NetworkRigidBodyComponent.AutoComponent.h>
#include <AzCore/Component/TransformBus.h>
#include <Multiplayer/Components/NetBindComponent.h>

namespace Physics
{
    class RigidBodyRequests;
}

namespace MultiplayerSample
{
    class NetworkRigidBodyComponent final : public NetworkRigidBodyComponentBase
    {
    public:
        AZ_MULTIPLAYER_COMPONENT(
            MultiplayerSample::NetworkRigidBodyComponent, s_networkRigidBodyComponentConcreteUuid, MultiplayerSample::NetworkRigidBodyComponentBase);

        static void Reflect(AZ::ReflectContext* context);

        NetworkRigidBodyComponent();

        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
        {
            required.push_back(AZ_CRC_CE("PhysXRigidBodyService"));
        }

        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& required)
        {
            required.push_back(AZ_CRC_CE("TransformService"));
            required.push_back(AZ_CRC_CE("PhysXRigidBodyService"));
        }

        void OnInit() override;
        void OnActivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
        void OnDeactivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;

    private:
        void OnTransformUpdate(const AZ::Transform& worldTm);
        void OnSyncRewind();

        Multiplayer::EntitySyncRewindEvent::Handler m_syncRewindHandler;
        AZ::TransformChangedEvent::Handler m_transformChangedHandler;
        Physics::RigidBodyRequests* m_physicsRigidBodyComponent = nullptr;
        Multiplayer::RewindableObject<AZ::Transform, Multiplayer::RewindHistorySize> m_transform;
    };

} // namespace MultiplayerSample
