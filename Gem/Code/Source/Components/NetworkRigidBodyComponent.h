/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
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
