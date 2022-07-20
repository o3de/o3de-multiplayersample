/*
 * Copyright (c) Contributors to the Open 3D Engine Project
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/Component/Component.h>
#include <AzFramework/Physics/Common/PhysicsSimulatedBody.h>
#include <AzFramework/Physics/Common/PhysicsSimulatedBodyEvents.h>

namespace MultiplayerSample
{
    /**
     * @brief Transports entities that collide with it to a fixed location.
     * 
     */
    class NetworkTeleportComponent
        : public AZ::Component
    {
    public:
        AZ_COMPONENT(NetworkTeleportComponent,
            "{917a6318-e047-4ec5-b6ed-bc95f74bd287}");

        static void Reflect(AZ::ReflectContext* reflection);

        NetworkTeleportComponent();

        void Activate() override;
        void Deactivate() override {};

    private:
        AzPhysics::SimulatedBodyEvents::OnTriggerEnter::Handler m_enterTrigger;
        void OnTriggerEnter(
            AzPhysics::SimulatedBodyHandle bodyHandle, const AzPhysics::TriggerEvent& triggerEvent);

        AZ::EntityId m_reset;

        AZ::Entity* GetCollidingEntity(AzPhysics::SimulatedBody* collidingBody) const;
        AZ::Vector3 GetDestination() const;
        void TeleportPlayer(const AZ::Vector3& vector, AZ::Entity* entity);
    };
}