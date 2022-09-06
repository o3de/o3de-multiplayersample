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
        }
    }

    void GemComponent::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        m_clientAnimationEvent.RemoveFromQueue();
        m_networkLocationHandler.Disconnect();
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

    void GemComponentController::HandleCollectedByPlayer([[maybe_unused]] AzNetworking::IConnection* invokingConnection)
    {
        // Hide the gem by moving it far away from the players' interest area.
        // This removes the gem from the clients' view. See @sv_ClientAwarenessRadius.
        GetNetworkTransformComponentController()->SetTranslation(AZ::Vector3::CreateAxisZ(-1000.f));
    }
}
