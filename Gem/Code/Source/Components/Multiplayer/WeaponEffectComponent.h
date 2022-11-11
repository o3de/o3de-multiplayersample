/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <Components/NetworkWeaponsComponent.h>
#include <Source/AutoGen/WeaponEffectComponent.AutoComponent.h>

namespace MultiplayerSample
{
    class WeaponEffectComponent
        : public WeaponEffectComponentBase
    {
    public:
        AZ_MULTIPLAYER_COMPONENT(MultiplayerSample::WeaponEffectComponent, s_weaponEffectComponentConcreteUuid, MultiplayerSample::WeaponEffectComponentBase);

        static void Reflect(AZ::ReflectContext* context);

        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
        {
            WeaponEffectComponentBase::GetRequiredServices(required);
            required.push_back(AZ_CRC("PopcornFXEmitterService"));
        }
        
        void OnActivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
        void OnDeactivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
        
        void PlayParticleEffect(const AZ::Vector3& start, const AZ::Vector3& end);

    private:
        void OnWeaponActivate(const WeaponActivationInfo& info);
        OnWeaponActivateEvent::Handler m_weaponActivateHandler{ [this](const WeaponActivationInfo& info)
        {
            OnWeaponActivate(info);
        } };

        void OnWeaponConfirmHit(const WeaponHitInfo& info);
        OnWeaponConfirmHitEvent::Handler m_weaponConfirmHandler{ [this](const WeaponHitInfo& info)
        {
            OnWeaponConfirmHit(info);
        } };
    };

    class WeaponEffectComponentController
        : public WeaponEffectComponentControllerBase
    {
    public:
        explicit WeaponEffectComponentController(WeaponEffectComponent& parent);

        void OnActivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
        void OnDeactivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;

    private:
        void OnWeaponPredictHit(const WeaponHitInfo& info);
        OnWeaponPredictHitEvent::Handler m_weaponPredictHandler{ [this](const WeaponHitInfo& info)
        {
            OnWeaponPredictHit(info);
        } };
    };
}
