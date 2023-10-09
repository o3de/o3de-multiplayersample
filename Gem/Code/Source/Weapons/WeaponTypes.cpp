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

#if AZ_TRAIT_CLIENT
#   include <PopcornFX/PopcornFXBus.h>
#endif

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
                editContext->Enum<SoundEffect>("SoundEffect", "Valid sound effects to use within MultiplayerSample")
                    ->Value("Unused", SoundEffect::Unused)
                    ->Value("PlayerFootSteps", SoundEffect::PlayerFootSteps)
                    ->Value("PlayerKnockedDown", SoundEffect::PlayerKnockedDown)
                    ->Value("ArmorBreaking", SoundEffect::ArmorBreaking)
                    ->Value("ArmorMend", SoundEffect::ArmorMend)
                    ->Value("PlayerOuch", SoundEffect::PlayerOuch)
                    ->Value("LadderClimb", SoundEffect::LadderClimb)
                    ->Value("ShutDown", SoundEffect::ShutDown)
                    ->Value("CountDown", SoundEffect::CountDown)
                    ->Value("GemPickup", SoundEffect::GemPickup)
                    ->Value("VictoryFanfare", SoundEffect::VictoryFanfare)
                    ->Value("LosingFanfare", SoundEffect::LosingFanfare)
                    ->Value("RoundStart", SoundEffect::RoundStart)
                    ->Value("RoundEnd", SoundEffect::RoundEnd)
                    ->Value("GameEnd", SoundEffect::GameEnd)
                    ->Value("LaserPistolMuzzleFlash", SoundEffect::LaserPistolMuzzleFlash)
                    ->Value("LaserPistolImpact", SoundEffect::LaserPistolImpact)
                    ->Value("BubbleGunBuildup", SoundEffect::BubbleGunBuildup)
                    ->Value("BubbleGunMuzzleFlash", SoundEffect::BubbleGunMuzzleFlash)
                    ->Value("BubbleGunProjectile", SoundEffect::BubbleGunProjectile)
                    ->Value("BubbleGunImpact", SoundEffect::BubbleGunImpact)
                    ->Value("JumpPadLaunch", SoundEffect::JumpPadLaunch)
                    ->Value("TeleporterUse", SoundEffect::TeleporterUse)
                    ->Value("EnergyBallTrapRisingOutOfTheGround", SoundEffect::EnergyBallTrapRisingOutOfTheGround)
                    ->Value("EnergyBallTrapBuildup", SoundEffect::EnergyBallTrapBuildup)
                    ->Value("EnergyBallTrapProjectile", SoundEffect::EnergyBallTrapProjectile)
                    ->Value("EnergyBallTrapImpact", SoundEffect::EnergyBallTrapImpact)
                    ->Value("EnergyBallTrapOnCooldown", SoundEffect::EnergyBallTrapOnCooldown);

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
                ->Field("EditorCollisionGroupId", &GatherParams::m_collisionGroupId)
                ->Field("Sphere", &GatherParams::m_sphere)
                ->Field("Box", &GatherParams::m_box)
                ->Field("Capsule", &GatherParams::m_capsule)
            ;

            AZ::EditContext* editContext = serializeContext->GetEditContext();
            if (editContext)
            {
                editContext->Class<GatherParams>("GatherParams", "Parameters that control entity gathers on weapon or projectile activates")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->DataElement(AZ::Edit::UIHandlers::ComboBox, &GatherParams::m_gatherShape, "GatherShape", "The shape of the primitive to use for intersect queries during gathers")
                        ->Attribute(AZ::Edit::Attributes::ChangeNotify, AZ::Edit::PropertyRefreshLevels::EntireTree)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &GatherParams::m_sphere, "Sphere", "Configuration of sphere shape")
                        ->Attribute(AZ::Edit::Attributes::Visibility, &GatherParams::IsSphereConfig)

                    ->DataElement(AZ::Edit::UIHandlers::Default, &GatherParams::m_box, "Box", "Configuration of box shape")
                        ->Attribute(AZ::Edit::Attributes::Visibility, &GatherParams::IsBoxConfig)

                    ->DataElement(AZ::Edit::UIHandlers::Default, &GatherParams::m_capsule, "Capsule", "Configuration of capsule shape")
                        ->Attribute(AZ::Edit::Attributes::Visibility, &GatherParams::IsCapsuleConfig)

                    ->DataElement(AZ::Edit::UIHandlers::Default, &GatherParams::m_castDistance, "CastDistance", "The cast distance or gather radius to use on hit or activate")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &GatherParams::m_castAngle, "CastAngle", "The cast/gather angle to use on hit or activate")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &GatherParams::m_travelSpeed, "TravelSpeed", "The 'speed' the cast should travel at for weapons that require target leading, 0 == instant hit (not projectile speed for projectile weapons!)")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &GatherParams::m_multiHit, "Multihit", "If true, the gather will not stop at the first entity hit, and will continue gathering entities until blocked by blocker geo")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &GatherParams::m_bulletDrop, "BulletDrop", "If true, the gather shape will follow a parabolic arc simulating gravity")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &GatherParams::m_collisionGroupId, "CollisionGroup", "The collision group hit mask for this weapon")
                ;
            }
        }
    }

    bool GatherParams::IsSphereConfig() const
    {
        return m_gatherShape == GatherShape::Sphere;
    }

    bool GatherParams::IsBoxConfig() const
    {
        return m_gatherShape == GatherShape::Box;
    }

    bool GatherParams::IsCapsuleConfig() const
    {
        return m_gatherShape == GatherShape::Capsule;
    }

    const Physics::ShapeConfiguration* GatherParams::GetCurrentShapeConfiguration() const 
    {
        switch(m_gatherShape)
        {
        case GatherShape::Box:
            return &m_box;
        case GatherShape::Sphere:
            return &m_sphere;
        case GatherShape::Capsule:
            return &m_capsule;
        default:
            return nullptr;
        }
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

    void WeaponParams::Reflect(AZ::ReflectContext* context)
    {
        AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context);
        if (serializeContext)
        {
            serializeContext->Class<WeaponParams>()
                ->Version(6)
                ->Field("WeaponType", &WeaponParams::m_weaponType)
                ->Field("WeaponMaxAimDistance", &WeaponParams::m_weaponMaxAimDistance)
                ->Field("CooldownTimeMs", &WeaponParams::m_cooldownTimeMs)
                ->Field("AnimFlag", &WeaponParams::m_animFlag)
                ->Field("ActivateFx", &WeaponParams::m_activateFx)
                ->Field("ImpactFx", &WeaponParams::m_impactFx)
                ->Field("DamageFx", &WeaponParams::m_damageFx)
                ->Field("ProjectileAsset", &WeaponParams::m_projectileAsset)
                ->Field("GatherParams", &WeaponParams::m_gatherParams)
                ->Field("DamageEffect", &WeaponParams::m_damageEffect)
                ->Field("LocallyPredicted", &WeaponParams::m_locallyPredicted);

            AZ::EditContext* editContext = serializeContext->GetEditContext();
            if (editContext)
            {
                editContext->Class<WeaponParams>("WeaponParams", "Parameters that control the behaviour of a weapon")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->DataElement(AZ::Edit::UIHandlers::ComboBox, &WeaponParams::m_weaponType, "WeaponType", "The basic type of weapon")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &WeaponParams::m_weaponMaxAimDistance, "WeaponMaxAimDistance", "The maximum distance of a raycast to locate a target when firing.")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &WeaponParams::m_cooldownTimeMs, "CooldownTimeMs", "The number of milliseconds needed before the weapon can activate again")
                    ->DataElement(AZ::Edit::UIHandlers::ComboBox, &WeaponParams::m_animFlag, "AnimFlag", "The animation flag to raise on the character when starting a fire sequence")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &WeaponParams::m_activateFx, "ActivateFx", "The effect to play upon weapon activation")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &WeaponParams::m_impactFx, "ImpactFx", "The effect to play at the point of impact upon weapon hit. Played predictively for autonomous clients, and authoritatively for simulated clients")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &WeaponParams::m_damageFx, "DamageFx", "The effect to play for each hit entitiy. Played authoritatively only")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &WeaponParams::m_projectileAsset, "ProjectileAsset", "If a projectile weapon, the archetype asset name for projectile properties")
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
            && serializer.Serialize(m_hitNormal, "HitNormal")
            && serializer.Serialize(m_hitNetEntityId, "HitNetEntityId");
    }

    void HitEntity::Reflect(AZ::ReflectContext* context)
    {
        AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context);
        if (serializeContext)
        {
            serializeContext->Class<HitEntity>()
                ->Version(1)
                ->Field("HitPosition", &HitEntity::m_hitPosition)
                ->Field("HitNormal", &HitEntity::m_hitNormal)
                ->Field("HitNetEntityId", &HitEntity::m_hitNetEntityId);
        }

        AZ::BehaviorContext* behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context);
        if (behaviorContext)
        {
            behaviorContext->Class<HitEntity>("HitEntity")
                ->Attribute(AZ::Script::Attributes::Scope, AZ::Script::Attributes::ScopeFlags::Common)
                ->Attribute(AZ::Script::Attributes::Module, "multiplayersample")
                ->Attribute(AZ::Script::Attributes::Category, "MultiplayerSample")
                ->Constructor<>()
                ->Property("HitPosition", BehaviorValueProperty(&HitEntity::m_hitPosition))
                ->Property("HitNormal", BehaviorValueProperty(&HitEntity::m_hitNormal))
                ->Property("HitNetEntityId", BehaviorValueProperty(&HitEntity::m_hitNetEntityId))
                ;
        }
    }

    bool HitEvent::operator!=(const HitEvent& rhs) const
    {
        if ((m_target != rhs.m_target) ||
            (m_shooterNetEntityId != rhs.m_shooterNetEntityId) ||
            (m_projectileNetEntityId != rhs.m_projectileNetEntityId) ||
            (m_hitEntities.size() != rhs.m_hitEntities.size()))
        {
            return true;
        }

        // We define equality here as having the same entries in the same order.
        for (size_t index = 0; index < m_hitEntities.size(); index++)
        {
            if ((m_hitEntities[index].m_hitNetEntityId != rhs.m_hitEntities[index].m_hitNetEntityId) ||
                (!m_hitEntities[index].m_hitPosition.IsClose(rhs.m_hitEntities[index].m_hitPosition)) ||
                (!m_hitEntities[index].m_hitNormal.IsClose(rhs.m_hitEntities[index].m_hitNormal)))
            {
                return true;
            }
        }

        return false;
    }

    bool HitEvent::Serialize(AzNetworking::ISerializer& serializer)
    {
        return serializer.Serialize(m_target, "Target")
            && serializer.Serialize(m_shooterNetEntityId, "ShooterNetEntityId")
            && serializer.Serialize(m_projectileNetEntityId, "ProjectileNetEntityId")
            && serializer.Serialize(m_hitEntities, "HitEntities");
    }

    void HitEvent::Reflect(AZ::ReflectContext* context)
    {
        AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context);
        if (serializeContext)
        {
            serializeContext->Class<HitEvent>()
                ->Version(1)
                ->Field("Target", &HitEvent::m_target)
                ->Field("ShooterNetEntityId", &HitEvent::m_shooterNetEntityId)
                ->Field("ProjectileNetEntityId", &HitEvent::m_projectileNetEntityId)
                ->Field("HitEntities", &HitEvent::m_hitEntities);
        }

        AZ::BehaviorContext* behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context);
        if (behaviorContext)
        {
            behaviorContext->Class<HitEvent>("HitEvent")
                ->Attribute(AZ::Script::Attributes::Scope, AZ::Script::Attributes::ScopeFlags::Common)
                ->Attribute(AZ::Script::Attributes::Module, "multiplayersample")
                ->Attribute(AZ::Script::Attributes::Category, "MultiplayerSample")
                ->Constructor<>()
                ->Property("Target", BehaviorValueProperty(&HitEvent::m_target))
                ->Property("ShooterNetEntityId", BehaviorValueProperty(&HitEvent::m_shooterNetEntityId))
                ->Property("ProjectileNetEntityId", BehaviorValueProperty(&HitEvent::m_projectileNetEntityId))
                ->Property("HitEntities", BehaviorValueProperty(&HitEvent::m_hitEntities))
                ;
        }
    }

    bool FireParams::operator!=(const FireParams& rhs) const
    {
        return !m_targetPosition.IsClose(rhs.m_targetPosition)
            || m_targetId != rhs.m_targetId;
    }

    bool FireParams::Serialize(AzNetworking::ISerializer& serializer)
    {
        return serializer.Serialize(m_sourcePosition, "SourcePosition")
            && serializer.Serialize(m_targetPosition, "TargetPosition")
            && serializer.Serialize(m_targetId, "TargetId");
    }
}
