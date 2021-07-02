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

#include <Source/Weapons/WeaponTypes.h>

namespace MultiplayerSample
{
    const char* GetEnumString(WeaponType value)
    {
        switch (value)
        {
        case WeaponType::None:
            return "None";
        case WeaponType::Trace:
            return "Trace";
        case WeaponType::Projectile:
            return "Projectile";
        }
        return "UNKNOWN";
    }

    const char* GetEnumString(WeaponStatus value)
    {
        switch (value)
        {
        case WeaponStatus::Idle:
            return "Idle";
        case WeaponStatus::Firing:
            return "Firing";
        }
        return "UNKNOWN";
    }

    const char* GetEnumString(GatherShape value)
    {
        switch (value)
        {
        case GatherShape::Point:
            return "Point";
        case GatherShape::Box:
            return "Box";
        case GatherShape::Sphere:
            return "Sphere";
        case GatherShape::Cylinder:
            return "Cylinder";
        case GatherShape::Capsule:
            return "Capsule";
        }
        return "UNKNOWN";
    }

    const char* GetEnumString(GatherDirection value)
    {
        switch (value)
        {
        case GatherDirection::TargetDir:
            return "TargetDir";
        case GatherDirection::ParentDir:
            return "ParentDir";
        }
        return "UNKNOWN";
    }

    const char* GetEnumString(EffectDirection value)
    {
        switch (value)
        {
        case EffectDirection::None:
            return "None";
        case EffectDirection::WeaponDirection:
            return "WeaponDirection";
        case EffectDirection::EntityDirection:
            return "EntityDirection";
        }
        return "UNKNOWN";
    }

    bool ClientEffect::Serialize(AzNetworking::ISerializer& serializer)
    {
        return serializer.Serialize(m_effectName, "EffectName")
            && serializer.Serialize(m_lifespan, "Lifespan")
            && serializer.Serialize(m_travelToTarget, "TravelToTarget")
            && serializer.Serialize(m_effectDirection, "EffectDirection");
    }

    void ClientEffect::Reflect([[maybe_unused]] AZ::ReflectContext* context)
    {
    }

    bool GatherParams::Serialize(AzNetworking::ISerializer& serializer)
    {
        return serializer.Serialize(m_gatherShape, "GatherShape")
            && serializer.Serialize(m_castDistance, "CastDistance")
            && serializer.Serialize(m_castAngle, "CastAngle")
            && serializer.Serialize(m_travelSpeed, "TravelSpeed")
            && serializer.Serialize(m_multiHit, "Multihit")
            && serializer.Serialize(m_ignoresGravity, "IgnoresGravity")
            && serializer.Serialize(m_hitMask, "HitMask");
    }

    void GatherParams::Reflect([[maybe_unused]] AZ::ReflectContext* context)
    {
    }

    bool HitEffect::Serialize(AzNetworking::ISerializer& serializer)
    {
        return serializer.Serialize(m_hitMagnitude, "HitMagnitude")
            && serializer.Serialize(m_hitFalloff, "HitFalloff")
            && serializer.Serialize(m_hitExponent, "HitExponent");
    }

    void HitEffect::Reflect([[maybe_unused]] AZ::ReflectContext* context)
    {
    }

    bool WeaponParams::Serialize(AzNetworking::ISerializer& serializer)
    {
        return serializer.Serialize(m_weaponType, "WeaponType")
            && serializer.Serialize(m_locallyPredicted, "LocallyPredicted")
            && serializer.Serialize(m_cooldownTimeMs, "CooldownTimeMs")
            && serializer.Serialize(m_animFlag, "AnimFlag")
            && serializer.Serialize(m_activateFx, "ActivateFx")
            && serializer.Serialize(m_impactFx, "ImpactFx")
            && serializer.Serialize(m_damageFx, "DamageFx")
            && serializer.Serialize(m_projectileAsset, "ProjectileAsset")
            && serializer.Serialize(m_ammoMaterialType, "AmmoMaterialType")
            && serializer.Serialize(m_gatherParams, "GatherParams")
            && serializer.Serialize(m_damageEffect, "DamageEffect");
    }

    void WeaponParams::Reflect([[maybe_unused]] AZ::ReflectContext* context)
    {
    }

    bool ActiveShot::operator!=(const ActiveShot& rhs) const
    {
        return !m_initialTransform.IsClose(rhs.m_initialTransform)
            || !m_targetPosition.IsClose(rhs.m_targetPosition)
            || !AZ::IsClose(m_lifetimeSeconds, rhs.m_lifetimeSeconds);
    }

    bool ActiveShot::Serialize(AzNetworking::ISerializer& serializer)
    {
        return serializer.Serialize(m_initialTransform, "InitialTransform")
            && serializer.Serialize(m_targetPosition, "TargetPosition")
            && serializer.Serialize(m_lifetimeSeconds, "LifetimeSeconds");
    }

    void ActiveShot::Reflect([[maybe_unused]] AZ::ReflectContext* context)
    {
    }

    bool WeaponState::operator!=(const WeaponState& rhs) const
    {
        return m_activationCount != rhs.m_activationCount
            || !AZ::IsClose(m_cooldownTime, rhs.m_cooldownTime)
            || m_status != rhs.m_status;
            //|| m_activeShots != rhs.m_activeShots;
    }

    bool WeaponState::Serialize(AzNetworking::ISerializer& serializer)
    {
        return serializer.Serialize(m_activationCount, "ActivationCount")
            && serializer.Serialize(m_cooldownTime, "CooldownTime")
            && serializer.Serialize(m_status, "Status")
            && serializer.Serialize(m_activeShots, "ActiveShots");
    }

    void WeaponState::Reflect([[maybe_unused]] AZ::ReflectContext* context)
    {
    }

    bool ActivateEvent::Serialize(AzNetworking::ISerializer& serializer)
    {
        return serializer.Serialize(m_initialTransform, "InitialTransform")
            && serializer.Serialize(m_targetPosition, "TargetPosition")
            && serializer.Serialize(m_shooterId, "ShooterId")
            && serializer.Serialize(m_projectileId, "ProjectileId");
    }

    void ActivateEvent::Reflect([[maybe_unused]] AZ::ReflectContext* context)
    {
    }

    bool HitEntity::Serialize(AzNetworking::ISerializer& serializer)
    {
        return serializer.Serialize(m_hitPosition, "HitPosition")
            && serializer.Serialize(m_hitNetEntityId, "HitNetEntityId");
    }

    void HitEntity::Reflect([[maybe_unused]] AZ::ReflectContext* context)
    {
    }

    bool HitEvent::Serialize(AzNetworking::ISerializer& serializer)
    {
        return serializer.Serialize(m_hitTransform, "HitTransform")
            && serializer.Serialize(m_shooterNetEntityId, "ShooterNetEntityId")
            && serializer.Serialize(m_hitEntities, "HitEntities");
    }

    void HitEvent::Reflect([[maybe_unused]] AZ::ReflectContext* context)
    {
    }

    bool FireParams::operator!=(const FireParams& rhs) const
    {
        return m_targetPosition.IsClose(rhs.m_targetPosition)
            || m_targetId != rhs.m_targetId;
    }

    bool FireParams::Serialize(AzNetworking::ISerializer& serializer)
    {
        return serializer.Serialize(m_targetPosition, "TargetPosition")
            && serializer.Serialize(m_targetId, "TargetId");
    }

    void FireParams::Reflect([[maybe_unused]] AZ::ReflectContext* context)
    {
    }
}
