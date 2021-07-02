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

#pragma once

#include <Source/Weapons/WeaponTypes.h>

namespace MultiplayerSample
{
    struct WeaponActivationInfo;
    struct WeaponHitInfo;

    //! @class WeaponListener
    //! @brief Listener class for IWeapon events.
    class WeaponListener
    {
    public:
        //! Called whenever a weapon activates.
        //! @param activationInfo details of the weapon activation
        virtual void OnActivate(const WeaponActivationInfo& activationInfo) = 0;

        //! Invoked when the weapon predictively hits a target entity.
        //! @param hitInfo details of the weapon hit
        virtual void OnPredictHit(const WeaponHitInfo& hitInfo) = 0;

        //! Invoked when the weapon gets confirmation from the server that it hit a target entity.
        //! @param hitInfo details of the weapon hit
        virtual void OnConfirmHit(const WeaponHitInfo& hitInfo) = 0;
    };

    //! @class IWeapon
    //! @brief Common interface for all multiplayer sample weapons, trace and projectile
    class IWeapon
    {
    public:

        IWeapon() = default;
        virtual ~IWeapon() = default;

        //! Returns the weapon index for this weapon instance.
        //! @return the weapon index for this weapon instance
        virtual WeaponIndex GetWeaponIndex() const = 0;

        //! Retrieves the WeaponParams for this IWeapon instance.
        //! @return the WeaponParams for the given IWeapon instance
        virtual const WeaponParams& GetParams() const = 0;

        //! Update the weapon's internal state.
        //! @param weaponState the weapon state being updated
        //! @param deltaTime   the amount of time to update weapon state by
        virtual void UpdateWeaponState(WeaponState& weaponState, float deltaTime) = 0;

        //! Returns whether the weapon believes it is able to perform its next StartFire or Activation event.
        //! @param weaponState    the weapon state being updated
        //! @param requiredStatus the weapon state required for this check to succeed
        //! @return boolean true if the weapon could enter firing mode or activate
        virtual bool CanStartNextEvent(const WeaponState& weaponState, WeaponStatus requiredStatus) const = 0;

        //! Called to begin weapon firing
        //! @param weaponState the weapon state being updated
        //! @param fireParams  the firing parameters to use for this activation sequence
        //! @return boolean true if the weapon successfully entered firing mode
        virtual bool TryStartFire(WeaponState& weaponState, const FireParams& fireParams) = 0;

        //! Retrieves the internal fire params
        //! @return structure containing the internal fire params
        virtual const FireParams& GetFireParams() const = 0;

        //! Sets the internal fire params
        //! @fireParams structure containing the internal fire params
        virtual void SetFireParams(const FireParams& fireParams) = 0;

        //! Returns the activate effect bound to this weapon instance.
        //! @return reference to the activate effect bound to this weapon instance
        virtual const ClientEffect& GetActivateEffect() const = 0;

        //! Returns the impact effect bound to this weapon instance.
        //! @return reference to the impact effect bound to this weapon instance
        virtual const ClientEffect& GetImpactEffect() const = 0;

        //! Returns the damage effect bound to this weapon instance.
        //! @return reference to the damage effect bound to this weapon instance
        virtual const ClientEffect& GetDamageEffect() const = 0;

        //! Returns the ammo type surface index for this weapon.
        //! @return the ammo type surface index for this weapon
        virtual int32_t GetAmmoTypeSurfaceIndex() const = 0;
    };

    //! @struct WeaponActivationInfo
    //! @brief Contains details for a single weapon activation.
    struct WeaponActivationInfo
    {
        //! Full constructor.
        //! @param weapon        reference to the weapon instance which activated
        //! @param activateEvent specific details about the weapon activation event
        WeaponActivationInfo(const IWeapon& weapon, const ActivateEvent& activateEvent);

        const IWeapon& m_weapon;        //< Reference to the weapon instance which activated
        ActivateEvent  m_activateEvent; //< Specific details about the weapon activation

        WeaponActivationInfo& operator =(const WeaponActivationInfo&) = delete; // Don't allow copying
    };

    //! @struct ServerHitInfo
    //! @brief Contains details for a single weapon hit on a server.
    struct WeaponHitInfo
    {
        //! Full constructor.
        //! @param a_Weapon       reference to the weapon instance which produced the hit
        //! @param a_GatherOrigin the origin point for any gather operations (center of explosion, muzzle of lasergun, etc..)
        //! @param a_HitEvent     specific details about the weapon hit event
        WeaponHitInfo(const IWeapon& weapon, const AZ::Vector3& gatherOrigin, const HitEvent& hitEvent);

        const IWeapon& m_weapon;       //< Reference to the weapon instance which produced the hit
        AZ::Vector3    m_gatherOrigin; //< Origin point for any gather operations (center of explosion, muzzle of lasergun, etc..)
        HitEvent       m_hitEvent;     //< Specific details about the weapon hit event

        WeaponHitInfo& operator =(const WeaponHitInfo&) = delete; // Don't allow copying, these guys get dispatched under special conditions
    };
}
