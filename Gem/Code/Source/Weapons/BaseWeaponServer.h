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

#include "Weapons/BaseWeapon.h"


namespace NovaNet
{
    // Forwards
    class  BaseWeaponServer;
    struct ServerActivationInfo;
    struct ServerHitInfo;


    /**
     * @class BaseWeaponServer
     * @brief Listener class for BaseWeaponServer events
     */
    class BaseWeaponServerListener
    {
    public:

        /**
         * Called whenever a weapon activates on the server
         * 
         * @param a_ActivationInfo details of the weapon activation
         */
        virtual void OnActivate(const ServerActivationInfo& a_ActivationInfo) = 0;

        /**
         * Internally called whenever the weapon hits a target entity
         * 
         * @param a_HitInfo details of the weapon hit
         */
        virtual void OnHit(const ServerHitInfo& a_HitInfo) = 0;

    };


    /**
     * @class BaseWeaponServer
     * @brief Base Server nova weapons class
     */
    class BaseWeaponServer
    :   public BaseWeapon
    {
    public:

        /**
         * Constructor
         * 
         * @param a_ConstructParams required construction parameters for the weapon instance
         * @param a_Listener        listener for BaseWeaponServer event callbacks
         */
        BaseWeaponServer(const BaseWeapon::ConstructParams& a_ConstructParams, BaseWeaponServerListener& a_Listener);

        /**
         * Internally called on every weapon activation
         * 
         * @param a_DeltaTime        the time in seconds this activate event corresponds to
         * @param a_WeaponState      the weapons internal state structure
         * @param a_WeaponOwner      the weapons owning entity
         * @param a_EventData        contains details of the activation event
         * @param a_bForceSkipGather if true, skip the gather step of the activation
         */
        virtual void ActivateServer(float a_DeltaTime, WeaponState& a_WeaponState, EntityPtr a_WeaponOwner, ActivateEvent& a_EventData, bool a_bForceSkipGather) = 0;

        /**
         * IWeapon interface
         * Returns the ammo type surface index for this weapon
         * 
         * @return the ammo type surface index for this weapon
         */
        virtual int32_t GetAmmoTypeSurfaceIndex() const AZ_OVERRIDE;

    protected:

        void DispatchHitEvents(const IntersectResults& a_GatherResults, const ActivateEvent& a_EventData);

        BaseWeaponServer& operator =(const BaseWeaponServer&) = delete;

        BaseWeaponServerListener& m_Listener;

    };


    /**
     * @struct ServerActivationInfo
     * @brief Contains details for a single weapon activation on a server
     */
    struct ServerActivationInfo
    {
        /**
         * Full constructor
         * 
         * @param a_Weapon        reference to the weapon instance which activated
         * @param a_ActivateEvent specific details about the weapon activation event
         */
        ServerActivationInfo(const IWeapon& a_Weapon, const ActivateEvent& a_ActivateEvent);

        const IWeapon& m_Weapon;        //< Reference to the weapon instance which activated
        ActivateEvent  m_ActivateEvent; //< Specific details about the weapon activation

        ServerActivationInfo& operator =(const ServerActivationInfo&) = delete; // Don't allow copying
    };


    /**
     * @struct ServerHitInfo
     * @brief Contains details for a single weapon hit on a server
     */
    struct ServerHitInfo
    {
        /**
         * Full constructor
         * 
         * @param a_Weapon       reference to the weapon instance which produced the hit
         * @param a_GatherOrigin the origin point for any gather operations (center of explosion, muzzle of lasergun, etc..)
         * @param a_HitEvent     specific details about the weapon hit event
         */
        ServerHitInfo(const IWeapon& a_Weapon, const Vec3& a_GatherOrigin, const HitEvent& a_HitEvent);

        const IWeapon& m_Weapon;       //< Reference to the weapon instance which produced the hit
        Vec3           m_GatherOrigin; //< Origin point for any gather operations (center of explosion, muzzle of lasergun, etc..)
        HitEvent       m_HitEvent;     //< Specific details about the weapon hit event

        ServerHitInfo& operator =(const ServerHitInfo&) = delete; // Don't allow copying, these guys get dispatched under special conditions
    };


    /**
     * Server weapons factory, creates a server weapon instance of the requested type
     * 
     * @param a_ConstructionParams construction parameters for the weapon instance
     * @param a_Listener           listener for BaseWeaponServer event callbacks
     * 
     * @return pointer to the new weapon instance, caller assumes strict ownership
     */
    BaseWeaponServer* Create(const BaseWeapon::ConstructParams& a_ConstructionParams, BaseWeaponServerListener& a_Listener);
}
