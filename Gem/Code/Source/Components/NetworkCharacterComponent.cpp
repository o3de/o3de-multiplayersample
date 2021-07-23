/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <Source/Components/NetworkCharacterComponent.h>
#include <AzFramework/Physics/CharacterBus.h>
#include <AzFramework/Physics/Character.h>
#include <Multiplayer/Components/NetworkTransformComponent.h>
#include <Multiplayer/NetworkTime/INetworkTime.h>

namespace MultiplayerSample
{
    void NetworkCharacterComponent::NetworkCharacterComponent::Reflect(AZ::ReflectContext* context)
    {
        AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context);
        if (serializeContext)
        {
            serializeContext->Class<NetworkCharacterComponent, NetworkCharacterComponentBase>()
                ->Version(1);
        }
        NetworkCharacterComponentBase::Reflect(context);
    }

    NetworkCharacterComponent::NetworkCharacterComponent()
        : m_translationEventHandler([this](const AZ::Vector3& translation) { OnTranslationChangedEvent(translation); })
    {
        ;
    }

    void NetworkCharacterComponent::OnInit()
    {
        ;
    }

    void NetworkCharacterComponent::OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        Physics::CharacterRequests* characterRequests = Physics::CharacterRequestBus::FindFirstHandler(GetEntityId());
        m_physicsCharacter = (characterRequests != nullptr) ? characterRequests->GetCharacter() : nullptr;
        GetNetBindComponent()->AddEntitySyncRewindEventHandler(m_syncRewindHandler);

        if (!HasController())
        {
            GetNetworkTransformComponent()->TranslationAddEvent(m_translationEventHandler);
        }
    }

    void NetworkCharacterComponent::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        ;
    }

    void NetworkCharacterComponent::OnTranslationChangedEvent([[maybe_unused]] const AZ::Vector3& translation)
    {
        OnSyncRewind();
    }

    void NetworkCharacterComponent::OnSyncRewind()
    {
        if (m_physicsCharacter == nullptr)
        {
            return;
        }

        const AZ::Vector3 currPosition = m_physicsCharacter->GetBasePosition();
        if (!currPosition.IsClose(GetNetworkTransformComponent()->GetTranslation()))
        {
            uint32_t frameId = static_cast<uint32_t>(Multiplayer::GetNetworkTime()->GetHostFrameId());
            m_physicsCharacter->SetFrameId(frameId);
            //m_physicsCharacter->SetBasePosition(GetNetworkTransformComponent()->GetTranslation());
        }
    }

    NetworkCharacterComponentController::NetworkCharacterComponentController(NetworkCharacterComponent& parent)
        : NetworkCharacterComponentControllerBase(parent)
    {
        ;
    }

    void NetworkCharacterComponentController::OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        ;
    }

    void NetworkCharacterComponentController::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        ;
    }

    AZ::Vector3 NetworkCharacterComponentController::TryMoveWithVelocity(const AZ::Vector3& velocity, float deltaTime)
    {
        if ((GetParent().m_physicsCharacter == nullptr) || (velocity.GetLengthSq() <= 0.0f))
        {
            return GetEntity()->GetTransform()->GetWorldTranslation();
        }
        GetParent().m_physicsCharacter->AddVelocity(velocity);
        GetParent().m_physicsCharacter->ApplyRequestedVelocity(deltaTime);
        GetEntity()->GetTransform()->SetWorldTranslation(GetParent().m_physicsCharacter->GetBasePosition());
        AZLOG
        (
            NET_Movement,
            "Moved to position %f x %f x %f",
            GetParent().m_physicsCharacter->GetBasePosition().GetX(),
            GetParent().m_physicsCharacter->GetBasePosition().GetY(),
            GetParent().m_physicsCharacter->GetBasePosition().GetZ()
        );
        return GetEntity()->GetTransform()->GetWorldTranslation();
    }
}
