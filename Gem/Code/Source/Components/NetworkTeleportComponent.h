/*
 * Copyright (c) Contributors to the Open 3D Engine Project
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <Source/AutoGen/NetworkTeleportComponent.AutoComponent.h>
#include <AzFramework/Physics/Common/PhysicsSimulatedBody.h>
#include <AzFramework/Physics/Common/PhysicsSimulatedBodyEvents.h>

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
        void OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating) override {};
        void OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating) override {};
    };

    class NetworkTeleportComponentController
        : public NetworkTeleportComponentControllerBase
    {
    public:
        NetworkTeleportComponentController(NetworkTeleportComponent& parent);

        void OnActivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
        void OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating) override {};

    private:
        AzPhysics::SimulatedBodyEvents::OnTriggerEnter::Handler m_enterTrigger;
        void OnTriggerEnter(
            AzPhysics::SimulatedBodyHandle bodyHandle, const AzPhysics::TriggerEvent& triggerEvent);

        AZ::Entity* GetCollidingEntity(AzPhysics::SimulatedBody* collidingBody) const;
        AZ::Vector3 GetDestinationVector() const;
        void TeleportPlayer(const AZ::Vector3& vector, AZ::Entity* entity);
    };

} // namespace MultiplayerSample
