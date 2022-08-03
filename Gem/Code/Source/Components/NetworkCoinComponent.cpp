/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <Source/Components/NetworkCoinComponent.h>

#include <AzCore/Serialization/SerializeContext.h>
#include <Multiplayer/Components/NetworkTransformComponent.h>

namespace MultiplayerSample
{
    void NetworkCoinComponent::Reflect(AZ::ReflectContext* context)
    {
        AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context);
        if (serializeContext)
        {
            serializeContext->Class<NetworkCoinComponent, NetworkCoinComponentBase>()
                ->Version(1);
        }
        NetworkCoinComponentBase::Reflect(context);
    }

    void NetworkCoinComponent::OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        if (IsNetEntityRoleClient())
        {
            m_rootLocation = GetEntity()->GetTransform()->GetWorldTranslation();
            GetNetworkTransformComponent()->TranslationAddEvent(m_networkLocationHandler);

            m_clientAnimationEvent.Enqueue(AZ::Time::ZeroTimeMs, true);
        }
    }

    void NetworkCoinComponent::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        m_clientAnimationEvent.RemoveFromQueue();
        m_networkLocationHandler.Disconnect();
    }

    void NetworkCoinComponent::ClientAnimationTick()
    {
        m_lifetime += m_clientAnimationEvent.TimeInQueueMs();

        auto updatedLocation = m_rootLocation;
        updatedLocation.SetZ(updatedLocation.GetZ() + 0.5f * GetVerticalAmplitude() * AZStd::sin(
            AZ::TimeMsToSeconds(m_lifetime) * AZ::Constants::TwoPi / GetVerticalBouncePeriod()));
        GetEntity()->GetTransform()->SetWorldTranslation(updatedLocation);

        const AZ::Quaternion rotation = AZ::Quaternion::CreateRotationZ(AZ::TimeMsToSeconds(m_lifetime) * GetAngularTurnSpeed());
        GetEntity()->GetTransform()->SetWorldRotationQuaternion(rotation);
    }

    void NetworkCoinComponent::OnNetworkLocationChanged(const AZ::Vector3& location)
    {
        m_rootLocation = location;
    }

    NetworkCoinComponentController::NetworkCoinComponentController(NetworkCoinComponent& parent)
        : NetworkCoinComponentControllerBase(parent)
    {
    }

    void NetworkCoinComponentController::OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
    }

    void NetworkCoinComponentController::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
    }
}
