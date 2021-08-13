/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <Source/MultiplayerSampleTypes.h>
#include <Multiplayer/MultiplayerTypes.h>
#include <AzCore/RTTI/TypeSafeIntegral.h>

namespace MultiplayerSample
{
    AZ_TYPE_SAFE_INTEGRAL(WeaponIndex, uint8_t);

    constexpr uint32_t MaxWeaponsPerComponent = 2; // The maximum number of weapons that can be attached to a single NetworkWeaponsComponent
    constexpr uint32_t MaxActiveShots = 32; // Maximum number of concurrently shots active for a single weapon
    constexpr uint32_t MaxHitEntities = 48; // Maximum number of entities that can be hit by a single shot

    // WeaponActivationBitset
    // Bitset used to represent which weapons have been activated for a specific input frame
    using WeaponActivationBitset = AzNetworking::FixedSizeBitset<MaxWeaponsPerComponent, uint8_t>;

    //! Types of weapons.
    enum class WeaponType
    {
        None,
        Trace,
        Projectile
    };
    const char* GetEnumString(WeaponType value);

    //! Status states for a multiplayersample weapon.
    enum class WeaponStatus
    {
        Idle,
        Firing
    };
    const char* GetEnumString(WeaponStatus value);

    //! Different primitive shape types that can be used for a hit gather.
    enum class GatherShape
    {
        Point,    // Gather entities that intersect a line-cast ending at the first hit
        Box,      // Gather entities that intersect a line-cast
        Sphere,   // Gather entities that fall within a spherical radius
        Cylinder, // Gather entities that intersect with a cylinder
        Capsule   // Gather entities that intersect with a capsule
    };
    const char* GetEnumString(GatherShape value);

    //! Different types of entity gather directions.
    enum class GatherDirection
    {
        TargetDir, // Gather entities offset from the target direction
        ParentDir  // Gather entities in the direction the parent entity is facing
    };
    const char* GetEnumString(GatherDirection value);

    //! Set of weapon effect orientations
    enum class EffectDirection
    {
        None,            // Use default orientation for effect playback
        WeaponDirection, // Use the orientation of the weapon activation for effect playback
        EntityDirection  // Use the orientation of the firing or hit entity for effect playback
    };
    const char* GetEnumString(EffectDirection value);

    void ReflectWeaponEnums(AZ::ReflectContext* context);

    using AssetStringType = AZStd::string; //< @TODO, Replace with proper asset reference types

    // @TODO: This needs to be hooked up to O3DE's effect system once it comes online
    //! Parameters that control client effect spawning.
    struct ClientEffect
    {
        AZ_RTTI(ClientEffect, "{B0B4E78C-51EC-4103-BC57-4C54ED36E3DB}");

        AssetStringType m_effectName;  // The effect to play upon weapon hit confirmation
        float m_lifespan = 1.0f;       // The lifespan value to provide the effects manager
        bool m_travelToTarget = false; // If true, effect will travel from origin to target position over it's lifetime
        EffectDirection m_effectDirection = EffectDirection::None; // The orientation to use when spawning the effect

        bool Serialize(AzNetworking::ISerializer& serializer);
        static void Reflect(AZ::ReflectContext* context);
    };

    //! Parameters that control entity gathers on weapon or projectile activates.
    struct GatherParams
    {
        AZ_RTTI(GatherParams, "{A20999EE-8A32-4C85-B93B-FFBD0D795A58}");

        GatherShape m_gatherShape;     // The shape of the primitive to use for intersect queries during gathers
        float m_castDistance = 1.0f;   // The cast distance or gather radius to use on hit or activate
        float m_castAngle = 0.0f;      // The cast/gather angle to use on hit or activate
        float m_travelSpeed = 0.0f;    // The 'speed' the cast should travel at for weapons that require target leading, 0 == instant hit (not projectile speed for projectile weapons!)
        bool m_multiHit = false;       // If true, the gather will not stop at the first entity hit, and will continue gathering entities until blocked by blocker geo
        bool m_bulletDrop = true;      // If true, the gather shape will follow a parabolic arc simulating gravity
        uint64_t m_hitMask = 0;        // The hit mask for this weapon (@TODO: What's the physics filter type with the new physics API?)

        bool Serialize(AzNetworking::ISerializer& serializer);
        static void Reflect(AZ::ReflectContext* context);
    };

    //! Parameters controlling hit effect application and falloff, HitMagnitude * ((HitFalloff * (1 - Distance / MaxDistance)) ^ HitExponent).
    //! Note that if you were wanting to implement melee mechanics, you might want additional attributes about stuns, knockbacks, etc.. here
    struct HitEffect
    {
        AZ_RTTI(HitEffect, "{24233666-5726-4DDA-8CB5-6859CFC4F7C2}");

        float m_hitMagnitude = 0.0f; // Base status amount to apply to hit entities
        float m_hitFalloff   = 1.0f; // Distance scalar to apply to hit entities
        float m_hitExponent  = 0.0f; // Falloff exponent to apply to hit entities

        bool Serialize(AzNetworking::ISerializer& serializer);
        static void Reflect(AZ::ReflectContext* context);
    };

    //! Parameters that control the behaviour of a weapon.
    struct WeaponParams
    {
        AZ_RTTI(WeaponParams, "{935FCBEB-F636-4D30-AB85-A1B225EA953F}");

        WeaponType m_weaponType = WeaponType::None; // The type of this weapon
        AZ::TimeMs m_cooldownTimeMs = AZ::TimeMs{ 0 }; // The number of milliseconds needed before the weapon can activate again
        CharacterAnimState m_animFlag = CharacterAnimState::Shooting; // The animation flag to raise on the network animation when firing this weapon
        AssetStringType m_activateFx;       // The effect to play upon weapon activation
        AssetStringType m_impactFx;         // The effect to play at the point of impact upon weapon hit. Played predictively for autonomous clients, and authoritatively for simulated clients
        AssetStringType m_damageFx;         // The effect to play for each hit entitiy. Played authoritatively only
        AssetStringType m_projectileAsset;  // If a projectile weapon, the prefab asset name for the projectile entity
        AssetStringType m_ammoMaterialType; // The effects material type of the ammo for bullet decals and other material effects (@TODO: Requires a replacement for the material effects system)
        GatherParams m_gatherParams;        // The type of gather to perform for trace weapons
        HitEffect m_damageEffect;           // Parameters controlling damage distribution on hit
        bool m_locallyPredicted = true;     // Whether or not this weapon is locally predicted or waits round trip to display on a client

        bool Serialize(AzNetworking::ISerializer& serializer);
        static void Reflect(AZ::ReflectContext* context);
    };

    using LifetimeSec = AzNetworking::QuantizedValues<1, 2, 0, 120>; // 2 minute max lifetime for any bullet

    //! Data to track a single active shot (trace weapons, not projectiles).
    struct ActiveShot
    {
        AZ::Transform m_initialTransform; // Transform of the weapon generating the activate event
        AZ::Vector3 m_targetPosition;     // Target location of the activate event
        LifetimeSec m_lifetimeSeconds;    // The number of seconds this shot has been alive for

        bool operator!=(const ActiveShot& rhs) const;
        bool Serialize(AzNetworking::ISerializer& serializer);
    };
    using ActiveShots = AZStd::fixed_vector<ActiveShot, MaxActiveShots>;

    //! Internal state data for a weapon instance (This structure is predictive and should not replicate to clients).
    struct WeaponState
    {
        uint8_t m_activationCount = 0; // The number of activations for this weapon (rolls over)
        float m_cooldownTime = 0.0f;   // The number of seconds before the next activation can occur
        WeaponStatus m_status = WeaponStatus::Idle; // The current weapon state
        ActiveShots m_activeShots;     // Vector of active shots still being tracked by this weapon

        bool operator!=(const WeaponState& rhs) const;
        bool Serialize(AzNetworking::ISerializer& serializer);
    };

    //! Structure containing details for a single weapon activation.
    struct ActivateEvent
    {
        AZ::Transform m_initialTransform = AZ::Transform::CreateIdentity();        // Transform of the weapon generating the activate event
        AZ::Vector3 m_targetPosition = AZ::Vector3::CreateZero();                  // Target location of the activate event
        Multiplayer::NetEntityId m_shooterId = Multiplayer::InvalidNetEntityId;    // NetEntityId of the shooter
        Multiplayer::NetEntityId m_projectileId = Multiplayer::InvalidNetEntityId; // NetEntityId of the projectile, or InvalidNetEntityId if a trace weapon

        bool Serialize(AzNetworking::ISerializer& serializer);
    };

    //! Single hit entity in a weapon hit event.
    struct HitEntity
    {
        AZ::Vector3 m_hitPosition = AZ::Vector3::CreateZero(); // Location where the entity was hit, NOT the location of the projectile or weapon in the case of area damage
        Multiplayer::NetEntityId m_hitNetEntityId = Multiplayer::InvalidNetEntityId; // Entity Id of the entity which was hit

        bool Serialize(AzNetworking::ISerializer& serializer);
    };
    using HitEntities = AZStd::fixed_vector<HitEntity, MaxHitEntities>;

    //! Structure containing details for a single weapon hit event.
    struct HitEvent
    {
        AZ::Transform m_hitTransform = AZ::Transform::CreateIdentity(); // Transform of the hit event, NOT the location of the entity that was hit in the case of area damage
        Multiplayer::NetEntityId m_shooterNetEntityId    = Multiplayer::InvalidNetEntityId; // Entity Id of the shooter
        Multiplayer::NetEntityId m_projectileNetEntityId = Multiplayer::InvalidNetEntityId; // Entity Id of the projectile, InvalidNetEntityId if this was a trace weapon hit
        HitEntities m_hitEntities; // Information about the entities that were hit

        bool Serialize(AzNetworking::ISerializer& serializer);
    };

    //! Structure containing details for a single fire event.
    struct FireParams
    {
        AZ::Vector3 m_targetPosition = AZ::Vector3::CreateZero(); // Location of the activate event.
        Multiplayer::NetEntityId m_targetId = Multiplayer::InvalidNetEntityId; // Entity Id of the target (for homing weapons)

        bool operator!=(const FireParams& rhs) const;
        bool Serialize(AzNetworking::ISerializer& serializer);
    };
}

namespace AZ
{
    AZ_TYPE_INFO_SPECIALIZE(MultiplayerSample::WeaponType, "{46FBBE01-25CE-4F25-82C1-8CCC95F3700F}");
    AZ_TYPE_INFO_SPECIALIZE(MultiplayerSample::WeaponStatus, "{7AF237A0-E3FD-4742-92C3-1ED82D4E0546}");
    AZ_TYPE_INFO_SPECIALIZE(MultiplayerSample::GatherShape, "{A10A1338-C08F-498E-815C-6410ADF7DFEC}");
    AZ_TYPE_INFO_SPECIALIZE(MultiplayerSample::GatherDirection, "{FB7C2937-E19F-47B5-9870-DDBCB9B39D18}");
    AZ_TYPE_INFO_SPECIALIZE(MultiplayerSample::EffectDirection, "{AF585331-AF80-4CEE-BE6C-55E9E21F0A88}");
}

AZ_TYPE_SAFE_INTEGRAL_SERIALIZEBINDING(MultiplayerSample::WeaponIndex);
