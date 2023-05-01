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

namespace MultiplayerSample
{
    AZ_CVAR(bool, gp_PauseOnWeaponGather, false, nullptr, AZ::ConsoleFunctorFlags::Null, "Will halt game execution on starting a weapon gather");

    BaseWeapon::BaseWeapon(const ConstructParams& constructParams)
        : m_owningEntity(constructParams.m_owningEntity)
        , m_weaponIndex(constructParams.m_weaponIndex)
        , m_weaponParams(constructParams.m_weaponParams)
        , m_weaponListener(constructParams.m_weaponListener)
    {
        m_activateEffect = constructParams.m_weaponParams.m_activateFx;
        m_activateEffect.Initialize();

        m_impactEffect = constructParams.m_weaponParams.m_impactFx;
        m_impactEffect.Initialize();

        m_damageEffect = constructParams.m_weaponParams.m_damageFx;
        m_damageEffect.Initialize();
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

    void BaseWeapon::ExecuteActivateEffect(const AZ::Transform& activateTransform, const AZ::Vector3& target) const
    {
        m_activateEffect.SetAttribute("Max Length", target.GetDistance(activateTransform.GetTranslation()));
        m_activateEffect.SetAttribute("Hit Position", target);
        m_activateEffect.TriggerEffect(activateTransform);
    }

    void BaseWeapon::ExecuteImpactEffect(const AZ::Vector3& activatePosition, const AZ::Vector3& hitPosition) const
    {
        const AZ::Transform hitTransform = AZ::Transform::CreateFromQuaternionAndTranslation(AZ::Quaternion::CreateIdentity(), hitPosition);
        m_impactEffect.SetAttribute("Hit Normal", (activatePosition - hitPosition).GetNormalized());
        m_impactEffect.SetAttribute("Hit Position", hitPosition);
        m_impactEffect.TriggerEffect(hitTransform);
    }

    void BaseWeapon::ExecuteDamageEffect(const AZ::Vector3& activatePosition, const AZ::Vector3& hitPosition) const
    {
        const AZ::Transform hitTransform = AZ::Transform::CreateFromQuaternionAndTranslation(AZ::Quaternion::CreateIdentity(), hitPosition);
        m_damageEffect.SetAttribute("Hit Normal", (activatePosition - hitPosition).GetNormalized());
        m_damageEffect.SetAttribute("Hit Position", hitPosition);
        m_damageEffect.TriggerEffect(hitTransform);
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
            AZ::Interface<AZ::IConsole>::Get()->PerformCommand("t_simulationTickScale 0");
        }
        return result;
    }

    ShotResult BaseWeapon::GatherEntitiesMultisegment(float deltaTime, ActiveShot& inOutActiveShot, IntersectResults& outResults)
    {
        ShotResult result = MultiplayerSample::GatherEntitiesMultisegment(m_weaponParams.m_gatherParams, m_gatheredNetEntityIds, deltaTime, inOutActiveShot, outResults);
        if (gp_PauseOnWeaponGather && (outResults.size() > 0))
        {
            AZ::Interface<AZ::IConsole>::Get()->PerformCommand("t_simulationTickScale 0");
        }
        return result;
    }

    void BaseWeapon::DispatchHitEvents(const IntersectResults& gatherResults, const ActivateEvent& eventData, const NetEntityIdSet& prefilteredNetEntityIds)
    {
        HitEvent hitEvent
        {
            eventData.m_targetPosition,
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

            hitEvent.m_hitEntities.emplace_back(HitEntity{ gatherResult.m_position, gatherResult.m_normal, gatherResult.m_netEntityId });
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
