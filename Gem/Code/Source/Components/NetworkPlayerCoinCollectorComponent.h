/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzFramework/Physics/Common/PhysicsEvents.h>
#include <Source/AutoGen/NetworkPlayerCoinCollectorComponent.AutoComponent.h>

namespace MultiplayerSample
{
    class NetworkPlayerCoinCollectorComponent
        : public NetworkPlayerCoinCollectorComponentBase
    {
    public:
        AZ_MULTIPLAYER_COMPONENT(MultiplayerSample::NetworkPlayerCoinCollectorComponent, s_networkPlayerCoinCollectorComponentConcreteUuid, MultiplayerSample::NetworkPlayerCoinCollectorComponentBase);

        static void Reflect(AZ::ReflectContext* context);

        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
        {
            required.push_back(AZ_CRC_CE("PhysicsCharacterControllerService"));
            NetworkPlayerCoinCollectorComponentBase::GetRequiredServices(required);
        }
        
        void OnActivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
        void OnDeactivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
    };

    class NetworkPlayerCoinCollectorComponentController
        : public NetworkPlayerCoinCollectorComponentControllerBase
    {
    public:
        explicit NetworkPlayerCoinCollectorComponentController(NetworkPlayerCoinCollectorComponent& parent);

        void OnActivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
        void OnDeactivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;

    private:
        void OnTriggerEvents(const AzPhysics::TriggerEventList& tel);
        AzPhysics::SceneEvents::OnSceneTriggersEvent::Handler m_trigger{[this](
            AzPhysics::SceneHandle, const AzPhysics::TriggerEventList& tel)
        {
            OnTriggerEvents(tel);
        }};
    };
}
