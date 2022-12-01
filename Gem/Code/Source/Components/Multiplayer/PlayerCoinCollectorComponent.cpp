/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <GameplayEffectsNotificationBus.h>
#include <PlayerCoinCollectorBus.h>
#include <UiCoinCountBus.h>
#include <AzCore/Component/ComponentApplicationBus.h>
#include <AzCore/Component/TransformBus.h>
#include <AzFramework/Physics/PhysicsScene.h>
#include <AzFramework/Physics/Collision/CollisionEvents.h>
#include <Components/Multiplayer/GemComponent.h>
#include <Source/Components/Multiplayer/PlayerCoinCollectorComponent.h>

namespace MultiplayerSample
{
    PlayerCoinCollectorComponentController::PlayerCoinCollectorComponentController(PlayerCoinCollectorComponent& parent)
        : PlayerCoinCollectorComponentControllerBase(parent)
    {
    }

    void PlayerCoinCollectorComponentController::OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        if (IsNetEntityRoleAuthority())
        {
            if (AzPhysics::SceneInterface* si = AZ::Interface<AzPhysics::SceneInterface>::Get())
            {
                const AzPhysics::SceneHandle sh = si->GetSceneHandle(AzPhysics::DefaultPhysicsSceneName);
                si->RegisterSceneTriggersEventHandler(sh, m_trigger);
            }
            PlayerCoinCollectorNotificationBus::Broadcast(&PlayerCoinCollectorNotifications::OnPlayerCollectorActivated, GetNetEntityId());
        }
        if (IsNetEntityRoleAutonomous())
        {
            CoinsCollectedAddEvent(m_coinCountChangedHandler);

            UiCoinCountNotificationBus::Broadcast(&UiCoinCountNotifications::OnCoinCountChanged, GetCoinsCollected());
        }
    }

    void PlayerCoinCollectorComponentController::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        if (IsNetEntityRoleAuthority())
        {
            PlayerCoinCollectorNotificationBus::Broadcast(&PlayerCoinCollectorNotifications::OnPlayerCollectorDeactivated, GetNetEntityId());
        }

        m_trigger.Disconnect();
        m_coinCountChangedHandler.Disconnect();
    }

    void PlayerCoinCollectorComponentController::OnTriggerEvents(const AzPhysics::TriggerEventList& tel)
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
                        if (GemComponent* gem = coinEntity->FindComponent<GemComponent>())
                        {
                            GameplayEffectsNotificationBus::Broadcast(&GameplayEffectsNotificationBus::Events::OnPositionalEffect,
                                SoundEffect::GemPickup, GetEntity()->GetTransform()->GetWorldTranslation());

                            gem->RPC_CollectedByPlayer();
                            ModifyCoinsCollected() += gem->GetGemScoreValue();
                            PlayerCoinCollectorNotificationBus::Broadcast(&PlayerCoinCollectorNotifications::OnPlayerCollectedCoinCountChanged, 
                                GetNetEntityId(), GetCoinsCollected());
                        }
                    }
                }
            }
        }
    }

    void PlayerCoinCollectorComponentController::OnCoinsChanged(uint16_t coins)
    {
        UiCoinCountNotificationBus::Broadcast(&UiCoinCountNotifications::OnCoinCountChanged, coins);
    }
}
