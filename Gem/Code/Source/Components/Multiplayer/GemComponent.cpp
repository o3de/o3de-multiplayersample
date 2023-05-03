/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AzCore/Serialization/SerializeContext.h>
#include <Components/Multiplayer/GemSpawnerComponent.h>
#include <Multiplayer/Components/NetworkTransformComponent.h>
#include <Source/Components/Multiplayer/GemComponent.h>

namespace MultiplayerSample
{
    void GemComponent::Reflect(AZ::ReflectContext* context)
    {
        AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context);
        if (serializeContext)
        {
            serializeContext->Class<GemComponent, GemComponentBase>()
                ->Version(1);
        }
        GemComponentBase::Reflect(context);
    }

    void GemComponent::OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        if (IsNetEntityRoleClient())
        {
            m_rootLocation = GetEntity()->GetTransform()->GetWorldTranslation();
            GetNetworkTransformComponent()->TranslationAddEvent(m_networkLocationHandler);

            // Tick on every frame.
            m_clientAnimationEvent.Enqueue(AZ::Time::ZeroTimeMs, true);

            // Physical bodies take time to enable after entity activation, so sign up for physics activation and disable it
            Physics::RigidBodyNotificationBus::Handler::BusConnect(GetEntityId());
        }
    }

    void GemComponent::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        Physics::RigidBodyNotificationBus::Handler::BusDisconnect();
        m_clientAnimationEvent.RemoveFromQueue();
        m_networkLocationHandler.Disconnect();
    }

    void GemComponent::OnPhysicsEnabled([[maybe_unused]] const AZ::EntityId& entityId)
    {
        // @entityId should be this entity since we signed up for it when calling
        //    Physics::RigidBodyNotificationBus::Handler::BusConnect

        Physics::RigidBodyNotificationBus::Handler::BusDisconnect();
        Physics::RigidBodyRequestBus::Event(GetEntityId(), &Physics::RigidBodyRequestBus::Events::DisablePhysics);
    }

    void GemComponent::ClientAnimationTick()
    {
        m_lifetime += m_clientAnimationEvent.TimeInQueueMs();

        auto updatedLocation = m_rootLocation;
        updatedLocation.SetZ(updatedLocation.GetZ() + 0.5f * GetVerticalAmplitude() * AZStd::sin(
            AZ::TimeMsToSeconds(m_lifetime + AZ::TimeMs{ GetRandomPeriodOffset() }) * AZ::Constants::TwoPi / GetVerticalBouncePeriod()));
        GetEntity()->GetTransform()->SetWorldTranslation(updatedLocation);

        const AZ::Quaternion rotation = AZ::Quaternion::CreateRotationZ(AZ::TimeMsToSeconds(
            m_lifetime + AZ::TimeMs{ GetRandomPeriodOffset() }) * GetAngularTurnSpeed());
        GetEntity()->GetTransform()->SetWorldRotationQuaternion(rotation);
    }

    void GemComponent::OnNetworkLocationChanged(const AZ::Vector3& location)
    {
        m_rootLocation = location;
    }

    GemComponentController::GemComponentController(GemComponent& parent)
        : GemComponentControllerBase(parent)
    {
    }

    void GemComponentController::OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
    }

    void GemComponentController::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
    }

#if AZ_TRAIT_SERVER
    void GemComponentController::HandleRPC_CollectedByPlayer([[maybe_unused]] AzNetworking::IConnection* invokingConnection)
    {
        if (m_controller)
        {
            m_controller->RemoveGem(GetEntity()->GetEntitySpawnTicketId());
            m_controller = nullptr;
        }
    }

    void GemComponentController::SetGemSpawnerController(GemSpawnerComponentController* controller)
    {
        m_controller = controller;
    }
#endif
}
