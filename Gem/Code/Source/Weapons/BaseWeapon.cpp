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
/*
        @TODO: Need a replacement fx and material fx system
        ArchetypeAssetLibrary* assetLibsInstance = ArchetypeAssetLibrary::GetInstance();
        NV_ASSERT(assetLibsInstance != nullptr, "Asset library is NULL");

        const ClientEffect* activateEffect = assetLibsInstance->GetArchetype<ClientEffect>(m_Params.GetActivateFx().c_str());
        const ClientEffect* impactEffect = assetLibsInstance->GetArchetype<ClientEffect>(m_Params.GetImpactFx().c_str());
        const ClientEffect* damageEffect = assetLibsInstance->GetArchetype<ClientEffect>(m_Params.GetDamageFx().c_str());

        m_ActivateEffect = activateEffect ? *activateEffect : ClientEffect();
        m_ImpactEffect = impactEffect ? *impactEffect : ClientEffect();
        m_DamageEffect = damageEffect ? *damageEffect : ClientEffect();
*/
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

    const ClientEffect& BaseWeapon::GetActivateEffect() const
    {
        return m_activateEffect;
    }

    const ClientEffect& BaseWeapon::GetImpactEffect() const
    {
        return m_impactEffect;
    }

    const ClientEffect& BaseWeapon::GetDamageEffect() const
    {
        return m_damageEffect;
    }

    int32_t BaseWeapon::GetAmmoTypeSurfaceIndex() const
    {
        return m_ammoSurfaceTypeIndex;
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
                if (prefilteredNetEntityIds.find(gatherResult.m_netEntityId) == prefilteredNetEntityIds.end())
                {
                    // Skip this hit, it was not gathered by the high-detail client physics trace, and should be filtered
                    continue;
                }
            }

            hitEvent.m_hitEntities.emplace_back(HitEntity{ gatherResult.m_position, gatherResult.m_netEntityId });
        }

        WeaponHitInfo hitInfo(*this, eventData.m_initialTransform.GetTranslation(), hitEvent);
        m_weaponListener.OnPredictHit(hitInfo);
    }

    WeaponActivationInfo::WeaponActivationInfo(const IWeapon& weapon, const ActivateEvent& activateEvent)
        : m_weapon(weapon)
        , m_activateEvent(activateEvent)
    {
        ;
    }

    WeaponHitInfo::WeaponHitInfo(const IWeapon& weapon, const AZ::Vector3& gatherOrigin, const HitEvent& hitEvent)
        : m_weapon(weapon)
        , m_gatherOrigin(gatherOrigin)
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
