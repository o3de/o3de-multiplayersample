/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <Source/Weapons/WeaponTypes.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/RTTI/BehaviorContext.h>
#include "AzFramework/Physics/CollisionBus.h"

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

    void ReflectWeaponEnums(AZ::ReflectContext* context)
    {
        AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context);
        if (serializeContext)
        {
            AZ::EditContext* editContext = serializeContext->GetEditContext();
            if (editContext)
            {
                editContext->Enum<CharacterAnimState>("CharacterAnimState", "Various MultiplayerSample character animation states")
                    ->Value("Idle", CharacterAnimState::Idle)
                    ->Value("Sprinting", CharacterAnimState::Sprinting)
                    ->Value("Crouching", CharacterAnimState::Crouching)
                    ->Value("Jumping", CharacterAnimState::Jumping)
                    ->Value("Falling", CharacterAnimState::Falling)
                    ->Value("Landing", CharacterAnimState::Landing)
                    ->Value("Climbing", CharacterAnimState::Climbing)
                    ->Value("Aiming", CharacterAnimState::Aiming)
                    ->Value("Shooting", CharacterAnimState::Shooting)
                    ->Value("Hit", CharacterAnimState::Hit)
                    ->Value("Dying", CharacterAnimState::Dying);

                editContext->Enum<WeaponType>("WeaponType", "Different weapon types in MultiplayerSample")
                    ->Value("None", WeaponType::None)
                    ->Value("Trace", WeaponType::Trace)
                    ->Value("Projectile", WeaponType::Projectile);

                editContext->Enum<WeaponStatus>("WeaponStatus", "Status states for a MultiplayerSample weapon")
                    ->Value("Idle", WeaponStatus::Idle)
                    ->Value("Firing", WeaponStatus::Firing);

                editContext->Enum<GatherShape>("GatherShape", "Different primitive shape types that can be used for a hit gather")
                    ->Value("Point", GatherShape::Point)
                    ->Value("Box", GatherShape::Box)
                    ->Value("Sphere", GatherShape::Sphere)
                    ->Value("Cylinder", GatherShape::Cylinder)
                    ->Value("Capsule", GatherShape::Capsule);

                editContext->Enum<GatherDirection>("GatherDirection", "Different types of entity gather directions")
                    ->Value("TargetDir", GatherDirection::TargetDir)
                    ->Value("ParentDir", GatherDirection::ParentDir);

                editContext->Enum<EffectDirection>("EffectDirection", "Set of weapon effect orientations")
                    ->Value("None", EffectDirection::None)
                    ->Value("WeaponDirection", EffectDirection::WeaponDirection)
                    ->Value("EntityDirection", EffectDirection::EntityDirection);
            }
        }
    }

    bool ClientEffect::Serialize(AzNetworking::ISerializer& serializer)
    {
        return serializer.Serialize(m_effectName, "EffectName")
            && serializer.Serialize(m_lifespan, "Lifespan")
            && serializer.Serialize(m_travelToTarget, "TravelToTarget")
            && serializer.Serialize(m_effectDirection, "EffectDirection");
    }

    void ClientEffect::Reflect(AZ::ReflectContext* context)
    {
        AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context);
        if (serializeContext)
        {
            serializeContext->Class<ClientEffect>()
                ->Version(1)
                ->Field("EffectName", &ClientEffect::m_effectName)
                ->Field("Lifespan", &ClientEffect::m_lifespan)
                ->Field("TravelToTarget", &ClientEffect::m_travelToTarget)
                ->Field("EffectDirection", &ClientEffect::m_effectDirection);

            AZ::EditContext* editContext = serializeContext->GetEditContext();
            if (editContext)
            {
                editContext->Class<ClientEffect>("ClientEffect", "Parameters that control client effect spawning")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &ClientEffect::m_effectName, "EffectName", "The effect to play upon weapon hit confirmation")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &ClientEffect::m_lifespan, "Lifespan", "The lifespan value to provide the effects manager")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &ClientEffect::m_travelToTarget, "TravelToTarget", "If true, effect will travel from origin to target position over it's lifetime")
                    ->DataElement(AZ::Edit::UIHandlers::ComboBox, &ClientEffect::m_effectDirection, "EffectDirection", "The orientation to use when spawning the effect")
                    ;
            }
        }
    }

    bool GatherParams::Serialize(AzNetworking::ISerializer& serializer)
    {
        return serializer.Serialize(m_gatherShape, "GatherShape")
            && serializer.Serialize(m_castDistance, "CastDistance")
            && serializer.Serialize(m_castAngle, "CastAngle")
            && serializer.Serialize(m_travelSpeed, "TravelSpeed")
            && serializer.Serialize(m_multiHit, "Multihit")
            && serializer.Serialize(m_bulletDrop, "BulletDrop")
            && serializer.Serialize(m_hitMask, "HitMask");
    }

    void GatherParams::Reflect(AZ::ReflectContext* context)
    {
        AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context);
        if (serializeContext)
        {
            serializeContext->Class<GatherParams>()
                ->Version(1)
                ->Field("GatherShape", &GatherParams::m_gatherShape)
                ->Field("CastDistance", &GatherParams::m_castDistance)
                ->Field("CastAngle", &GatherParams::m_castAngle)
                ->Field("TravelSpeed", &GatherParams::m_travelSpeed)
                ->Field("Multihit", &GatherParams::m_multiHit)
                ->Field("BulletDrop", &GatherParams::m_bulletDrop)
                ->Field("HitMask", &GatherParams::m_hitMask)
                ->Field("EditorCollisionGroupId", &GatherParams::m_editorCollisionGroupId)
            ;

            AZ::EditContext* editContext = serializeContext->GetEditContext();
            if (editContext)
            {
                editContext->Class<GatherParams>("GatherParams", "Parameters that control entity gathers on weapon or projectile activates")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->DataElement(AZ::Edit::UIHandlers::ComboBox, &GatherParams::m_gatherShape, "GatherShape", "The shape of the primitive to use for intersect queries during gathers")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &GatherParams::m_castDistance, "CastDistance", "The cast distance or gather radius to use on hit or activate")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &GatherParams::m_castAngle, "CastAngle", "The cast/gather angle to use on hit or activate")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &GatherParams::m_travelSpeed, "TravelSpeed", "The 'speed' the cast should travel at for weapons that require target leading, 0 == instant hit (not projectile speed for projectile weapons!)")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &GatherParams::m_multiHit, "Multihit", "If true, the gather will not stop at the first entity hit, and will continue gathering entities until blocked by blocker geo")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &GatherParams::m_bulletDrop, "BulletDrop", "If true, the gather shape will follow a parabolic arc simulating gravity")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &GatherParams::m_editorCollisionGroupId, "EditorCollisionGroupId", "The collision group hit mask for this weapon")
                        ->Attribute(AZ::Edit::Attributes::ChangeNotify, &GatherParams::OnCollisionGroupChanged)
                ;
            }
        }
    }

    void GatherParams::OnCollisionGroupChanged()
    {
        AzPhysics::CollisionGroup collisionGroup;
        Physics::CollisionRequestBus::BroadcastResult(
            collisionGroup, &Physics::CollisionRequests::GetCollisionGroupById, m_editorCollisionGroupId);
        m_hitMask = collisionGroup.GetMask();
    }

    bool HitEffect::Serialize(AzNetworking::ISerializer& serializer)
    {
        return serializer.Serialize(m_hitMagnitude, "HitMagnitude")
            && serializer.Serialize(m_hitFalloff, "HitFalloff")
            && serializer.Serialize(m_hitExponent, "HitExponent");
    }

    void HitEffect::Reflect(AZ::ReflectContext* context)
    {
        AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context);
        if (serializeContext)
        {
            serializeContext->Class<HitEffect>()
                ->Version(1)
                ->Field("HitMagnitude", &HitEffect::m_hitMagnitude)
                ->Field("HitFalloff", &HitEffect::m_hitFalloff)
                ->Field("HitExponent", &HitEffect::m_hitExponent);

            AZ::EditContext* editContext = serializeContext->GetEditContext();
            if (editContext)
            {
                editContext->Class<HitEffect>("HitEffect", "Parameters controlling hit effect application and falloff, HitMagnitude * ((HitFalloff * (1 - Distance / MaxDistance)) ^ HitExponent)")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &HitEffect::m_hitMagnitude, "HitMagnitude", "Base status amount to apply to hit entities")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &HitEffect::m_hitFalloff, "HitFalloff", "Distance scalar to apply to hit entities")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &HitEffect::m_hitExponent, "HitExponent", "Falloff exponent to apply to hit entities");
            }
        }
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

    void WeaponParams::Reflect(AZ::ReflectContext* context)
    {
        AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context);
        if (serializeContext)
        {
            serializeContext->Class<WeaponParams>()
                ->Version(1)
                ->Field("WeaponType", &WeaponParams::m_weaponType)
                ->Field("CooldownTimeMs", &WeaponParams::m_cooldownTimeMs)
                ->Field("AnimFlag", &WeaponParams::m_animFlag)
                ->Field("ActivateFx", &WeaponParams::m_activateFx)
                ->Field("ImpactFx", &WeaponParams::m_impactFx)
                ->Field("DamageFx", &WeaponParams::m_damageFx)
                ->Field("ProjectileAsset", &WeaponParams::m_projectileAsset)
                ->Field("AmmoMaterialType", &WeaponParams::m_ammoMaterialType)
                ->Field("GatherParams", &WeaponParams::m_gatherParams)
                ->Field("DamageEffect", &WeaponParams::m_damageEffect)
                ->Field("LocallyPredicted", &WeaponParams::m_locallyPredicted);

            AZ::EditContext* editContext = serializeContext->GetEditContext();
            if (editContext)
            {
                editContext->Class<WeaponParams>("WeaponParams", "Parameters that control the behaviour of a weapon")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->DataElement(AZ::Edit::UIHandlers::ComboBox, &WeaponParams::m_weaponType, "WeaponType", "The basic type of weapon")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &WeaponParams::m_cooldownTimeMs, "CooldownTimeMs", "The number of milliseconds needed before the weapon can activate again")
                    ->DataElement(AZ::Edit::UIHandlers::ComboBox, &WeaponParams::m_animFlag, "AnimFlag", "The animation flag to raise on the character when starting a fire sequence")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &WeaponParams::m_activateFx, "ActivateFx", "The effect to play upon weapon activation")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &WeaponParams::m_impactFx, "ImpactFx", "The effect to play at the point of impact upon weapon hit. Played predictively for autonomous clients, and authoritatively for simulated clients")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &WeaponParams::m_damageFx, "DamageFx", "The effect to play for each hit entitiy. Played authoritatively only")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &WeaponParams::m_projectileAsset, "ProjectileAsset", "If a projectile weapon, the archetype asset name for projectile properties")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &WeaponParams::m_ammoMaterialType, "AmmoMaterialType", "The material type name (out of GameSDK/libs/materialeffects/surfacetypes.xml) to use for driving impact effects")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &WeaponParams::m_gatherParams, "GatherParams", "The type of gather to perform for shape-cast weapons")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &WeaponParams::m_damageEffect, "DamageEffect", "The modifier parameters to apply to hit entities for damage")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &WeaponParams::m_locallyPredicted, "LocallyPredicted", "If true, autonomous clients predict activations and hits");
            }
        }
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

    bool ActivateEvent::Serialize(AzNetworking::ISerializer& serializer)
    {
        return serializer.Serialize(m_initialTransform, "InitialTransform")
            && serializer.Serialize(m_targetPosition, "TargetPosition")
            && serializer.Serialize(m_shooterId, "ShooterId")
            && serializer.Serialize(m_projectileId, "ProjectileId");
    }

    bool HitEntity::Serialize(AzNetworking::ISerializer& serializer)
    {
        return serializer.Serialize(m_hitPosition, "HitPosition")
            && serializer.Serialize(m_hitNetEntityId, "HitNetEntityId");
    }

    bool HitEvent::Serialize(AzNetworking::ISerializer& serializer)
    {
        return serializer.Serialize(m_hitTransform, "HitTransform")
            && serializer.Serialize(m_shooterNetEntityId, "ShooterNetEntityId")
            && serializer.Serialize(m_hitEntities, "HitEntities");
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
}
