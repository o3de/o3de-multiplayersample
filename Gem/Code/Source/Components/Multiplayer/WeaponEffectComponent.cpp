/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <GameplayEffectsNotificationBus.h>
#include <AzCore/Component/TransformBus.h>
#include <LmbrCentral/Audio/AudioTriggerComponentBus.h>
#include <Source/Components/Multiplayer/WeaponEffectComponent.h>

#if AZ_TRAIT_CLIENT
#include <PopcornFX/PopcornFXBus.h>
#endif

namespace MultiplayerSample
{
    void WeaponEffectComponent::Reflect(AZ::ReflectContext* context)
    {
        AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context);
        if (serializeContext)
        {
            serializeContext->Class<WeaponEffectComponent, WeaponEffectComponentBase>()
                ->Version(1);
        }
        WeaponEffectComponentBase::Reflect(context);
    }

    void WeaponEffectComponent::OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        GetNetworkWeaponsComponent()->AddOnWeaponActivateEventHandler(m_weaponActivateHandler);
        GetNetworkWeaponsComponent()->AddOnWeaponConfirmHitEventHandler(m_weaponConfirmHandler);
    }

    void WeaponEffectComponent::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        m_weaponActivateHandler.Disconnect();
        m_weaponConfirmHandler.Disconnect();
    }

    void WeaponEffectComponent::PlayParticleEffect(
        [[maybe_unused]] const AZ::Vector3& start, [[maybe_unused]] const AZ::Vector3& end)
    {
#if AZ_TRAIT_CLIENT
        if (PopcornFX::PopcornFXEmitterComponentRequests* particle =
            PopcornFX::PopcornFXEmitterComponentRequestBus::FindFirstHandler(GetEntityId()))
        {
            AZ::s32 attributeId = particle->GetAttributeId("Start");
            if (attributeId >= 0)
            {
                particle->SetAttributeAsFloat3(attributeId, start);
            }

            attributeId = particle->GetAttributeId("End");
            if (attributeId >= 0)
            {
                particle->SetAttributeAsFloat3(attributeId, end);
            }

            particle->Restart(true);
        }
#endif

        GameplayEffectsNotificationBus::Broadcast(&GameplayEffectsNotificationBus::Events::OnPositionalEffect, 
            SoundEffect::LaserPistolMuzzleFlash, GetEntity()->GetTransform()->GetWorldTranslation());
    }

    void WeaponEffectComponent::OnWeaponActivate(const WeaponActivationInfo& info)
    {
        // This is the locally predicated effect on the player that initiated the weapon.
        const AZ::Vector3 start = GetNetworkWeaponsComponent()->GetCurrentShotStartPosition();
        const AZ::Vector3& end = info.m_activateEvent.m_targetPosition;
        PlayParticleEffect(start, end);
    }

    void WeaponEffectComponent::OnWeaponConfirmHit([[maybe_unused]] const WeaponHitInfo& info)
    {
        const AZ::Vector3& hitCenter = info.m_hitEvent.m_hitTransform.GetTranslation();
        GameplayEffectsNotificationBus::Broadcast(&GameplayEffectsNotificationBus::Events::OnPositionalEffect,
            SoundEffect::LaserPistolImpact, hitCenter);
    }


    WeaponEffectComponentController::WeaponEffectComponentController(WeaponEffectComponent& parent)
        : WeaponEffectComponentControllerBase(parent)
    {
    }

    void WeaponEffectComponentController::OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        if (IsNetEntityRoleAutonomous() || IsNetEntityRoleAuthority())
        {
            GetParent().GetNetworkWeaponsComponent()->AddOnWeaponPredictHitEventHandler(m_weaponPredictHandler);
        }
    }

    void WeaponEffectComponentController::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        m_weaponPredictHandler.Disconnect();
    }

    void WeaponEffectComponentController::OnWeaponPredictHit([[maybe_unused]] const WeaponHitInfo& info)
    {
        /*
         * At the moment, the particle effect plays both the trace and the hit effect in one go when predicted on the originated player.
         * If the particle effects were separated, one could spawn an entity to play the hit effect only.
         * Then this call would play the predicted hit effect, while @OnWeaponActivate would only play the trace line effect.
         */
    }
}
