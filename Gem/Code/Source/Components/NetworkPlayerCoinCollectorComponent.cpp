/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AzCore/Serialization/SerializeContext.h>
#include <AzFramework/Physics/PhysicsScene.h>
#include <AzFramework/Physics/RigidBodyBus.h>
#include <AzFramework/Physics/Collision/CollisionEvents.h>
#include <AzFramework/Physics/Components/SimulatedBodyComponentBus.h>
#include <Source/Components/NetworkPlayerCoinCollectorComponent.h>

#pragma optimize("", off)

namespace MultiplayerSample
{
    void NetworkPlayerCoinCollectorComponent::Reflect(AZ::ReflectContext* context)
    {
        AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context);
        if (serializeContext)
        {
            serializeContext->Class<NetworkPlayerCoinCollectorComponent, NetworkPlayerCoinCollectorComponentBase>()
                ->Version(1);
        }
        NetworkPlayerCoinCollectorComponentBase::Reflect(context);
    }

    void NetworkPlayerCoinCollectorComponent::OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
    }

    void NetworkPlayerCoinCollectorComponent::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
    }

    NetworkPlayerCoinCollectorComponentController::NetworkPlayerCoinCollectorComponentController(NetworkPlayerCoinCollectorComponent& parent)
        : NetworkPlayerCoinCollectorComponentControllerBase(parent)
    {
    }

    void NetworkPlayerCoinCollectorComponentController::OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        auto* si = AZ::Interface<AzPhysics::SceneInterface>::Get();
        if (si != nullptr)
        {
            const AzPhysics::SceneHandle sh = si->GetSceneHandle(AzPhysics::DefaultPhysicsSceneName);
            si->RegisterSceneTriggersEventHandler(sh, m_trigger);
        }
    }

    void NetworkPlayerCoinCollectorComponentController::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        m_trigger.Disconnect();
    }

    void NetworkPlayerCoinCollectorComponentController::OnTriggerEvents(const AzPhysics::TriggerEventList& tel)
    {
        for (const AzPhysics::TriggerEvent& te : tel)
        {
            if (te.m_otherBody && te.m_otherBody->GetEntityId() == GetEntityId())
            {
                if (te.m_type == AzPhysics::TriggerEvent::Type::Enter)
                {
                    AZ::Entity* coinEntity = nullptr;
                    AZ::ComponentApplicationBus::BroadcastResult(coinEntity,
                        &AZ::ComponentApplicationBus::Events::FindEntity, te.m_triggerBody->GetEntityId());
                    if (coinEntity)
                    {
                        if (NetworkCoinComponent* coin = coinEntity->FindComponent<NetworkCoinComponent>())
                        {
                            coin->CollectedByPlayer();
                            ModifyCoinsCollected()++;
                        }
                    }
                }                
            }
        }
    }
}

#pragma optimize("", on)
