/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 * 
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <Source/Components/CharacterComponent.h>
#include <AzFramework/Physics/CharacterBus.h>
#include <AzFramework/Physics/Character.h>
#include <Multiplayer/Components/NetworkTransformComponent.h>
#include <Multiplayer/NetworkTime/INetworkTime.h>
#include <PhysXCharacters/API/CharacterController.h>
#include <PhysX/PhysXLocks.h>
#include <PhysX/Utils.h>
#include <Source/Components/NetworkRigidBodyComponent.h>

namespace MultiplayerSample
{
    bool CollisionLayerBasedControllerFilter(const physx::PxController& controllerA, const physx::PxController& controllerB)
    {
        PHYSX_SCENE_READ_LOCK(controllerA.getActor()->getScene());
        physx::PxRigidDynamic* actorA = controllerA.getActor();
        physx::PxRigidDynamic* actorB = controllerB.getActor();

        if (actorA && actorA->getNbShapes() > 0 && actorB && actorB->getNbShapes() > 0)
        {
            physx::PxShape* shapeA = nullptr;
            actorA->getShapes(&shapeA, 1, 0);
            physx::PxFilterData filterDataA = shapeA->getSimulationFilterData();
            physx::PxShape* shapeB = nullptr;
            actorB->getShapes(&shapeB, 1, 0);
            physx::PxFilterData filterDataB = shapeB->getSimulationFilterData();
            return PhysX::Utils::Collision::ShouldCollide(filterDataA, filterDataB);
        }

        return true;
    }

    physx::PxQueryHitType::Enum CollisionLayerBasedObjectPreFilter(
        const physx::PxFilterData& filterData,
        const physx::PxShape* shape,
        const physx::PxRigidActor* actor,
        [[maybe_unused]] physx::PxHitFlags& queryFlags)
    {
        // non-kinematic dynamic bodies should not impede the movement of the character
        if (actor->getConcreteType() == physx::PxConcreteType::eRIGID_DYNAMIC)
        {
            const physx::PxRigidDynamic* rigidDynamic = static_cast<const physx::PxRigidDynamic*>(actor);

            bool isKinematic = (rigidDynamic->getRigidBodyFlags() & physx::PxRigidBodyFlag::eKINEMATIC);
            if (isKinematic)
            {
                const PhysX::ActorData* actorData = PhysX::Utils::GetUserData(rigidDynamic);
                if (actorData)
                {
                    const AZ::EntityId entityId = actorData->GetEntityId();

                    if (NetworkRigidBodyRequestBus::FindFirstHandler(entityId) != nullptr)
                    {
                        // Network rigid bodies are kinematic on the client but dynamic on the server,
                        // hence filtering treats these actors as dynamic to support client prediction and avoid desyncs
                        isKinematic = false;
                    }
                }
            }

            if (!isKinematic)
            {
                return physx::PxQueryHitType::eNONE;
            }
        }

        // all other cases should be determined by collision filters
        if (PhysX::Utils::Collision::ShouldCollide(filterData, shape->getSimulationFilterData()))
        {
            return physx::PxQueryHitType::eBLOCK;
        }

        return physx::PxQueryHitType::eNONE;
    }

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

    CharacterComponent::CharacterComponent()
        : m_translationEventHandler([this](const AZ::Vector3& translation) { OnTranslationChangedEvent(translation); })
    {
        ;
    }

    void CharacterComponent::OnInit()
    {
        ;
    }

    void CharacterComponent::OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        Physics::CharacterRequests* characterRequests = Physics::CharacterRequestBus::FindFirstHandler(GetEntityId());
        m_physicsCharacter = (characterRequests != nullptr) ? characterRequests->GetCharacter() : nullptr;
        GetNetBindComponent()->AddEntitySyncRewindEventHandler(m_syncRewindHandler);

        if (m_physicsCharacter)
        {
            auto controller = static_cast<PhysX::CharacterController*>(m_physicsCharacter);
            controller->SetFilterFlags(physx::PxQueryFlag::eSTATIC | physx::PxQueryFlag::eDYNAMIC | physx::PxQueryFlag::ePREFILTER);
            if (auto callbackManager = controller->GetCallbackManager())
            {
                callbackManager->SetControllerFilter(CollisionLayerBasedControllerFilter);
                callbackManager->SetObjectPreFilter(CollisionLayerBasedObjectPreFilter);
            }
        }

        if (!HasController() || GetController()->IsAutonomous())
        {
            GetNetworkTransformComponent()->TranslationAddEvent(m_translationEventHandler);
        }
    }

    void CharacterComponent::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        ;
    }

    void CharacterComponent::OnTranslationChangedEvent([[maybe_unused]] const AZ::Vector3& translation)
    {
        if (m_physicsCharacter == nullptr)
        {
            return;
        }

        const AZ::Vector3 currPosition = m_physicsCharacter->GetBasePosition();
        if (!currPosition.IsClose(translation))
        {
            uint32_t frameId = static_cast<uint32_t>(Multiplayer::GetNetworkTime()->GetHostFrameId());
            m_physicsCharacter->SetFrameId(frameId);
            if (GetController() && GetController()->IsAutonomous())
            {
                m_physicsCharacter->SetBasePosition(translation);
                GetEntity()->GetTransform()->SetLocalTranslation(translation);
            }
        }
    }

    void CharacterComponent::OnSyncRewind()
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
        }
    }

    bool CharacterComponent::IsOnGround() const
    {
        auto pxController = static_cast<physx::PxController*>(m_physicsCharacter->GetNativePointer());
        if (!pxController)
        {
            return true;
        }

        physx::PxControllerState state;
        pxController->getState(state);
        return state.touchedActor != nullptr || (state.collisionFlags & physx::PxControllerCollisionFlag::eCOLLISION_DOWN) != 0;
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
