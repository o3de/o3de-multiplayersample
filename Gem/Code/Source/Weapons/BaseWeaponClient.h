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


// Fowards
class DebugComponentC;


namespace NovaNet
{
    // Forwards
    class  BaseWeaponClient;
    struct ClientActivationInfo;
    struct ClientHitInfo;


    /**
     * @class BaseWeaponClient
     * @brief Listener class for BaseWeaponClient events
     */
    class BaseWeaponClientListener
    {
    public:

        /**
         * Called whenever a weapon activates on the client
         * 
         * @param a_ActivationInfo details of the weapon activation
         */
        virtual void OnActivate(const ClientActivationInfo& a_ActivationInfo) = 0;

        /**
         * Invoked when the weapon predictively hits a target entity
         * 
         * @param a_HitInfo details of the weapon hit
         */
        virtual void OnPredictHit(const ClientHitInfo& a_HitInfo) = 0;

        /**
         * Invoked when the weapon gets confirmation from the server that it hit a target entity
         * 
         * @param a_HitInfo details of the weapon hit
         */
        virtual void OnConfirmHit(const ClientHitInfo& a_HitInfo) = 0;

    };


    /**
     * @class BaseWeaponClient
     * @brief Base client nova weapons class
     */
    class BaseWeaponClient
    :   public BaseWeapon
    {
    public:

        /**
         * Constructor
         * 
         * @param a_ConstructParams required construction parameters for the weapon instance
         * @param a_Listener        listener for BaseWeaponClient event callbacks
         */
        BaseWeaponClient(const BaseWeapon::ConstructParams& a_ConstructParams, BaseWeaponClientListener& a_Listener);

        /**
         * Internally called on every weapon activation
         * 
         * @param a_DeltaTime   the time in seconds this activate event corresponds to
         * @param a_WeaponState the weapons internal state structure
         * @param a_Predicted   if true, the weapon activation is predictive
         * @param a_Replay      if true, these activations are happening during replay due to a correction
         * @param a_EventData   contains details of the activation event
         */
        virtual void ActivateClient(float a_DeltaTime, WeaponState& a_WeaponState, bool a_Predicted, bool a_Replay, ActivateEvent& a_EventData) = 0;

        /**
         * IWeapon interface
         * Returns the ammo type surface index for this weapon
         * 
         * @return the ammo type surface index for this weapon
         */
        virtual int32_t GetAmmoTypeSurfaceIndex() const AZ_OVERRIDE;

    protected:

        void DispatchHitEvents(const IntersectResults& a_GatherResults, const ActivateEvent& a_EventData, const EntityIdSet& a_PrefilteredEntityIds);

        BaseWeaponClient& operator =(const BaseWeaponClient&) = delete;

        BaseWeaponClientListener& m_Listener;

        int32_t m_AmmoSurfaceTypeIndex;

    };


    /**
     * @struct ClientActivationInfo
     * @brief Contains details for a single weapon activation on a client
     */
    struct ClientActivationInfo
    {
        /**
         * Full constructor
         * 
         * @param a_Weapon        reference to the weapon instance which activated
         * @param a_ActivateEvent specific details about the weapon activation event
         */
        ClientActivationInfo(const IWeapon& a_Weapon, const ActivateEvent& a_ActivateEvent);

        const IWeapon& m_Weapon;        //< Reference to the weapon instance which activated
        ActivateEvent  m_ActivateEvent; //< Specific details about the weapon activation

        ClientActivationInfo& operator =(const ClientActivationInfo&) = delete; // Don't allow copying
    };


    /**
     * @struct ClientHitInfo
     * @brief Contains details for a single weapon hit on a client
     */
    struct ClientHitInfo
    {
        /**
         * Full constructor
         * 
         * @param a_Weapon   reference to the weapon instance which produced the hit
         * @param a_HitEvent specific details about the weapon hit event
         */
        ClientHitInfo(const IWeapon& a_Weapon, const HitEvent& a_HitEvent);

        const IWeapon& m_Weapon;   //< Reference to the weapon instance which produced the hit
        HitEvent       m_HitEvent; //< Specific details about the weapon hit event

        ClientHitInfo& operator =(const ClientHitInfo&) = delete; // Don't allow copying, these guys get dispatched under special conditions
    };


    /**
     * Client weapons factory, creates a client weapon instance of the requested type
     * 
     * @param a_ConstructionParams construction parameters for the weapon instance
     * @param a_Listener           listener for BaseWeaponClient event callbacks
     * 
     * @return pointer to the new weapon instance, caller assumes strict ownership
     */
    BaseWeaponClient* Create(const BaseWeapon::ConstructParams& a_ConstructionParams, BaseWeaponClientListener& a_Listener);
}
