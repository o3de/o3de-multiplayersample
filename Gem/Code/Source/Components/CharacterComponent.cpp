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

#include <Source/Components/CharacterComponent.h>
#include <Multiplayer/Components/NetworkTransformComponent.h>
#include <AzFramework/Physics/CharacterBus.h>
#include <AzFramework/Physics/Character.h>

namespace MultiplayerSample
{
    void CharacterComponent::CharacterComponent::Reflect(AZ::ReflectContext* context)
    {
        AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context);
        if (serializeContext)
        {
            serializeContext->Class<CharacterComponent, CharacterComponentBase>()
                ->Version(1);
        }
        CharacterComponentBase::Reflect(context);
    }

    void CharacterComponent::OnInit()
    {
        ;
    }

    void CharacterComponent::OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        m_physicsCharacter = Physics::CharacterRequestBus::FindFirstHandler(GetEntity()->GetId());
        GetNetBindComponent()->AddEntitySyncRewindEventHandler(m_syncRewindHandler);
    }

    void CharacterComponent::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        ;
    }

    void CharacterComponent::OnSyncRewind()
    {
        if (m_physicsCharacter == nullptr)
        {
            return;
        }

        const AZ::Vector3 currPosition = m_physicsCharacter->GetBasePosition();

        if (currPosition != GetNetworkTransformComponent()->GetTranslation())
        {
            m_physicsCharacter->SetBasePosition(GetNetworkTransformComponent()->GetTranslation());
        }
    }

    CharacterComponentController::CharacterComponentController(CharacterComponent& parent)
        : CharacterComponentControllerBase(parent)
    {
        ;
    }

    void CharacterComponentController::OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        ;
    }

    void CharacterComponentController::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        ;
    }

    AZ::Vector3 CharacterComponentController::TryMoveWithVelocity(const AZ::Vector3& velocity, float deltaTime)
    {
        if (GetParent().m_physicsCharacter == nullptr)
        {
            return GetNetworkTransformComponentController()->GetTranslation();
        }
        GetParent().m_physicsCharacter->AddVelocity(velocity);
        GetParent().m_physicsCharacter->GetCharacter()->ApplyRequestedVelocity(deltaTime);
        GetNetworkTransformComponentController()->SetTranslation(GetParent().m_physicsCharacter->GetBasePosition());
        AZLOG
        (
            NET_Movement,
            "Moved to position %f x %f x %f",
            GetParent().m_physicsCharacter->GetBasePosition().GetX(),
            GetParent().m_physicsCharacter->GetBasePosition().GetY(),
            GetParent().m_physicsCharacter->GetBasePosition().GetZ()
        );
        return GetNetworkTransformComponentController()->GetTranslation();
    }
}
