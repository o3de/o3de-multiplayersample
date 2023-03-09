/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <Source/Weapons/BaseWeapon.h>
#include <Source/Weapons/TraceWeapon.h>
#include <Source/Weapons/ProjectileWeapon.h>
#include <AzCore/Console/ILogger.h>

#if AZ_TRAIT_CLIENT
#   include <PopcornFX/PopcornFXBus.h>
#endif

namespace MultiplayerSample
{
    AZ_CVAR(bool, gp_PauseOnWeaponGather, false, nullptr, AZ::ConsoleFunctorFlags::Null, "Will halt game execution on starting a weapon gather");
    AZ_CVAR(bool, cl_KillEffectOnRestart, false, nullptr, AZ::ConsoleFunctorFlags::Null, "Controls whether or not to kill current effects on restart");

    BaseWeapon::BaseWeapon(const ConstructParams& constructParams)
        : m_owningEntity(constructParams.m_owningEntity)
        , m_weaponIndex(constructParams.m_weaponIndex)
        , m_weaponParams(constructParams.m_weaponParams)
        , m_weaponListener(constructParams.m_weaponListener)
    {
#if AZ_TRAIT_CLIENT
        if (PopcornFX::PopcornFXRequests* popcornFx = PopcornFX::PopcornFXRequestBus::FindFirstHandler())
        {
            const PopcornFX::SpawnParams params = PopcornFX::SpawnParams(true, false, AZ::Transform::CreateIdentity());
            m_activateEffect = popcornFx->SpawnEffectById(constructParams.m_weaponParams.m_activateAssetId, params);
            m_impactEffect = popcornFx->SpawnEffectById(constructParams.m_weaponParams.m_impactAssetId, params);
            m_damageEffect = popcornFx->SpawnEffectById(constructParams.m_weaponParams.m_damageAssetId, params);
        }

        if (Audio::IAudioSystem* audioSystem = AZ::Interface<Audio::IAudioSystem>::Get())
        {
            m_activateSoundProxy = audioSystem->GetAudioProxy();
            m_impactSoundProxy = audioSystem->GetAudioProxy();
            m_damageSoundProxy = audioSystem->GetAudioProxy();
            m_activateTriggerId = audioSystem->GetAudioTriggerID(constructParams.m_weaponParams.m_activateAudioTrigger.c_str());
            m_impactTriggerId = audioSystem->GetAudioTriggerID(constructParams.m_weaponParams.m_impactAudioTrigger.c_str());
            m_damageTriggerId = audioSystem->GetAudioTriggerID(constructParams.m_weaponParams.m_damageAudioTrigger.c_str());
        }
#endif
    }

    BaseWeapon::~BaseWeapon()
    {
#if AZ_TRAIT_CLIENT
        if (PopcornFX::PopcornFXRequests* popcornFx = PopcornFX::PopcornFXRequestBus::FindFirstHandler())
        {
            popcornFx->DestroyEffect(m_activateEffect);
            m_activateEffect = nullptr;
            popcornFx->DestroyEffect(m_impactEffect);
            m_impactEffect = nullptr;
            popcornFx->DestroyEffect(m_damageEffect);
            m_damageEffect = nullptr;
        }

        if (Audio::IAudioSystem* audioSystem = AZ::Interface<Audio::IAudioSystem>::Get())
        {
            audioSystem->RecycleAudioProxy(m_activateSoundProxy);
            m_activateSoundProxy = nullptr;
            audioSystem->RecycleAudioProxy(m_impactSoundProxy);
            m_impactSoundProxy = nullptr;
            audioSystem->RecycleAudioProxy(m_damageSoundProxy);
            m_damageSoundProxy = nullptr;
        }
#endif
    }

    WeaponIndex BaseWeapon::GetWeaponIndex() const
    {
        return m_weaponIndex;
    }

    const WeaponParams& BaseWeapon::GetParams() const
    {
        return m_weaponParams;
    }

    void BaseWeapon::UpdateWeaponState(WeaponState& weaponState, float deltaTime)
    {
        const float newCooldown = AZStd::max(0.0f, weaponState.m_cooldownTime - deltaTime);
        weaponState.m_cooldownTime = newCooldown;
        TickActiveShots(weaponState, deltaTime);
    }

    bool BaseWeapon::CanStartNextEvent(const WeaponState& weaponState, WeaponStatus requiredStatus) const
    {
        // In valid state?
        if (weaponState.m_status != requiredStatus)
        {
            return false;
        }
        return weaponState.m_cooldownTime <= 0.0f;
    }

    bool BaseWeapon::TryStartFire(WeaponState& weaponState, const FireParams& fireParams)
    {
        if (!CanStartNextEvent(weaponState, WeaponStatus::Idle))
        {
            AZLOG(NET_Weapons, "TryStartFire rejected; WeaponStatus is %s, cooldowntime is %f", GetEnumString(weaponState.m_status), weaponState.m_cooldownTime);
            return false;
        }

        weaponState.m_status = WeaponStatus::Firing;
        weaponState.m_cooldownTime = 0.0f;
        m_fireParams = fireParams;
        m_gatheredNetEntityIds.clear();
        m_gatheredNetEntityIds.insert(m_owningEntity.GetNetEntityId());
        return true;
    }

    const FireParams& BaseWeapon::GetFireParams() const
    {
        return m_fireParams;
    }

    void BaseWeapon::SetFireParams(const FireParams& fireParams)
    {
        m_fireParams = fireParams;
    }

    void BaseWeapon::ExecuteActivateEffect([[maybe_unused]] const AZ::Transform& activateTransform, [[maybe_unused]] const AZ::Vector3& target) const
    {
#if AZ_TRAIT_CLIENT
        if (m_activateEffect != nullptr)
        {
            if (PopcornFX::PopcornFXRequests* popcornFx = PopcornFX::PopcornFXRequestBus::FindFirstHandler())
            {
                popcornFx->EffectSetTransform(m_activateEffect, activateTransform);

                int32_t lengthAttrId = popcornFx->EffectGetAttributeId(m_activateEffect, "Max Length");
                if (lengthAttrId >= 0)
                {
                    const float length = target.GetDistance(activateTransform.GetTranslation());
                    popcornFx->EffectSetAttributeAsFloat(m_activateEffect, lengthAttrId, length);
                }

                int32_t hitAttrId = popcornFx->EffectGetAttributeId(m_activateEffect, "Hit Position");
                if (hitAttrId >= 0)
                {
                    popcornFx->EffectSetAttributeAsFloat3(m_activateEffect, hitAttrId, target);
                }

                popcornFx->EffectRestart(m_activateEffect, cl_KillEffectOnRestart);
            }
        }

        if ((m_activateSoundProxy != nullptr) && (m_activateTriggerId != INVALID_AUDIO_CONTROL_ID))
        {
            m_activateSoundProxy->SetPosition(activateTransform.GetTranslation());
            m_activateSoundProxy->ExecuteTrigger(m_activateTriggerId);
        }
#endif
    }

    void BaseWeapon::ExecuteImpactEffect([[maybe_unused]] const AZ::Vector3& activatePosition, [[maybe_unused]] const AZ::Vector3& hitPosition) const
    {
#if AZ_TRAIT_CLIENT
        if (m_impactEffect != nullptr)
        {
            if (PopcornFX::PopcornFXRequests* popcornFx = PopcornFX::PopcornFXRequestBus::FindFirstHandler())
            {
                const AZ::Transform hitTransform = AZ::Transform::CreateFromQuaternionAndTranslation(AZ::Quaternion::CreateIdentity(), hitPosition);
                popcornFx->EffectSetTransform(m_impactEffect, hitTransform);

                int32_t hitAttrId = popcornFx->EffectGetAttributeId(m_impactEffect, "Hit Position");
                if (hitAttrId >= 0)
                {
                    popcornFx->EffectSetAttributeAsFloat3(m_impactEffect, hitAttrId, hitPosition);
                }

                int32_t normalAttrId = popcornFx->EffectGetAttributeId(m_impactEffect, "Hit Normal");
                if (normalAttrId >= 0)
                {
                    const AZ::Vector3 hitNormal = (activatePosition - hitPosition).GetNormalized();
                    popcornFx->EffectSetAttributeAsFloat3(m_impactEffect, normalAttrId, hitNormal);
                }

                popcornFx->EffectRestart(m_impactEffect, cl_KillEffectOnRestart);
            }
        }

        if ((m_impactSoundProxy != nullptr) && (m_impactTriggerId != INVALID_AUDIO_CONTROL_ID))
        {
            m_impactSoundProxy->SetPosition(hitPosition);
            m_impactSoundProxy->ExecuteTrigger(m_impactTriggerId);
        }
#endif
    }

    void BaseWeapon::ExecuteDamageEffect([[maybe_unused]] const AZ::Vector3& activatePosition, [[maybe_unused]] const AZ::Vector3& hitPosition) const
    {
#if AZ_TRAIT_CLIENT
        if (m_damageEffect != nullptr)
        {
            if (PopcornFX::PopcornFXRequests* popcornFx = PopcornFX::PopcornFXRequestBus::FindFirstHandler())
            {
                const AZ::Transform hitTransform = AZ::Transform::CreateFromQuaternionAndTranslation(AZ::Quaternion::CreateIdentity(), hitPosition);
                popcornFx->EffectSetTransform(m_damageEffect, hitTransform);

                int32_t hitAttrId = popcornFx->EffectGetAttributeId(m_damageEffect, "Hit Position");
                if (hitAttrId >= 0)
                {
                    popcornFx->EffectSetAttributeAsFloat3(m_damageEffect, hitAttrId, hitPosition);
                }

                int32_t normalAttrId = popcornFx->EffectGetAttributeId(m_damageEffect, "Hit Normal");
                if (normalAttrId >= 0)
                {
                    const AZ::Vector3 hitNormal = (activatePosition - hitPosition).GetNormalized();
                    popcornFx->EffectSetAttributeAsFloat3(m_damageEffect, normalAttrId, hitNormal);
                }

                popcornFx->EffectRestart(m_damageEffect, cl_KillEffectOnRestart);
            }
        }

        if ((m_damageSoundProxy != nullptr) && (m_damageTriggerId != INVALID_AUDIO_CONTROL_ID))
        {
            m_damageSoundProxy->SetPosition(hitPosition);
            m_damageSoundProxy->ExecuteTrigger(m_damageTriggerId);
        }
#endif
    }

    bool BaseWeapon::ActivateInternal(WeaponState& weaponState, bool validateFiringState)
    {
        if (validateFiringState && !CanStartNextEvent(weaponState, WeaponStatus::Firing))
        {
            AZLOG(NET_Weapons, "ActivateInternal rejected! WeaponStatus is %s, cooldowntime is %f", GetEnumString(weaponState.m_status), weaponState.m_cooldownTime);
            return false;
        }

        weaponState.m_cooldownTime = static_cast<float>(m_weaponParams.m_cooldownTimeMs) * 0.001f;
        weaponState.m_status = WeaponStatus::Idle;
        ++weaponState.m_activationCount;
        return true;
    }

    bool BaseWeapon::GatherEntities(const ActivateEvent& eventData, IntersectResults& outResults)
    {
        const bool result = MultiplayerSample::GatherEntities(m_weaponParams.m_gatherParams, eventData, m_gatheredNetEntityIds, outResults);
        if (gp_PauseOnWeaponGather && (outResults.size() > 0))
        {
            AZ::Interface<AZ::IConsole>::Get()->PerformCommand("t_scale 0");
        }
        return result;
    }

    ShotResult BaseWeapon::GatherEntitiesMultisegment(float deltaTime, ActiveShot& inOutActiveShot, IntersectResults& outResults)
    {
        ShotResult result = MultiplayerSample::GatherEntitiesMultisegment(m_weaponParams.m_gatherParams, m_gatheredNetEntityIds, deltaTime, inOutActiveShot, outResults);
        if (gp_PauseOnWeaponGather && (outResults.size() > 0))
        {
            AZ::Interface<AZ::IConsole>::Get()->PerformCommand("t_scale 0");
        }
        return result;
    }

    void BaseWeapon::DispatchHitEvents(const IntersectResults& gatherResults, const ActivateEvent& eventData, const NetEntityIdSet& prefilteredNetEntityIds)
    {
        HitEvent hitEvent
        {
            AZ::Transform::CreateFromQuaternionAndTranslation(eventData.m_initialTransform.GetRotation(), eventData.m_targetPosition),
            eventData.m_shooterId,
            Multiplayer::InvalidNetEntityId,
            HitEntities()
        };

        for (auto gatherResult : gatherResults)
        {
            if (prefilteredNetEntityIds.size() > 0)
            {
                if (prefilteredNetEntityIds.find(gatherResult.m_netEntityId) != prefilteredNetEntityIds.end())
                {
                    // Skip this hit, it was not gathered by the high-detail client physics trace, and should be filtered
                    continue;
                }
            }

            hitEvent.m_hitEntities.emplace_back(HitEntity{ gatherResult.m_position, gatherResult.m_netEntityId });
        }

        WeaponHitInfo hitInfo(*this, hitEvent);
        m_weaponListener.OnWeaponHit(hitInfo);
    }

    WeaponActivationInfo::WeaponActivationInfo(const IWeapon& weapon, const ActivateEvent& activateEvent)
        : m_weapon(weapon)
        , m_activateEvent(activateEvent)
    {
        ;
    }

    WeaponHitInfo::WeaponHitInfo(const IWeapon& weapon, const HitEvent& hitEvent)
        : m_weapon(weapon)
        , m_hitEvent(hitEvent)
    {
        ;
    }

    AZStd::unique_ptr<IWeapon> CreateWeapon(const ConstructParams& constructParams)
    {
        switch (constructParams.m_weaponParams.m_weaponType)
        {
        case WeaponType::Trace:
            return AZStd::make_unique<TraceWeapon>(constructParams);
        case WeaponType::Projectile:
            return AZStd::make_unique<ProjectileWeapon>(constructParams);
        case WeaponType::None:
        default:
            break;
        }
        return nullptr;
    }
}
