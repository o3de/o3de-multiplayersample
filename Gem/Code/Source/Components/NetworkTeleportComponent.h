/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of
 * this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <Source/AutoGen/NetworkTeleportComponent.AutoComponent.h>
#include <AzFramework/Physics/Common/PhysicsSimulatedBody.h>
#include <AzFramework/Physics/Common/PhysicsSimulatedBodyEvents.h>
#include <AzFramework/Physics/RigidBodyBus.h>

namespace MultiplayerSample
{
    /**
     * @brief Transports entities that collide with it to a fixed location.
     * 
     */
    class NetworkTeleportComponent
        : public NetworkTeleportComponentBase
    {
    public:
        AZ_COMPONENT(NetworkTeleportComponent, "{917a6318-e047-4ec5-b6ed-bc95f74bd287}", 
            NetworkTeleportComponentBase);

        static void Reflect(AZ::ReflectContext* reflection);

        void OnInit() override {};
        void OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating) override;
        void OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating) override;

#if AZ_TRAIT_CLIENT
        void HandleNotifyTeleport(AzNetworking::IConnection* invokingConnection) override;
#endif

    private:
        GameEffect m_effect;
    };

    class NetworkTeleportComponentController
        : public NetworkTeleportComponentControllerBase
        , private Physics::RigidBodyNotificationBus::Handler
    {
    public:
        NetworkTeleportComponentController(NetworkTeleportComponent& parent);

        void OnActivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
        void OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating) override;

    private:
        void OnPhysicsEnabled(const AZ::EntityId& entityId) override;
        void OnPhysicsDisabled(const AZ::EntityId& entityId) override;

        AzPhysics::SimulatedBodyEvents::OnTriggerEnter::Handler m_enterTrigger;
        void OnTriggerEnter(
            AzPhysics::SimulatedBodyHandle bodyHandle, const AzPhysics::TriggerEvent& triggerEvent);

        AZ::Entity* GetCollidingEntity(AzPhysics::SimulatedBody* collidingBody) const;
        AZ::Vector3 GetDestinationVector() const;
        void TeleportPlayer(const AZ::Vector3& vector, AZ::Entity* entity);
    };

} // namespace MultiplayerSample
