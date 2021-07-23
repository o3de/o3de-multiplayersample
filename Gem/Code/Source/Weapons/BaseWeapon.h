/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <Source/Weapons/IWeapon.h>
#include <Source/Weapons/WeaponGathers.h>
#include <Multiplayer/NetworkEntity/NetworkEntityHandle.h>

namespace MultiplayerSample
{
    struct ConstructParams
    {
        const Multiplayer::ConstNetworkEntityHandle m_owningEntity; // the owning entity for this weapon
        const WeaponIndex m_weaponIndex;    // the weapon index
        const WeaponParams& m_weaponParams; // weapon behaviour parameters
        WeaponListener& m_weaponListener;   // the listener for weapon events
    };

    //! @class BaseWeapon
    //! @brief Common base weapon class.
    class BaseWeapon
        : public IWeapon
    {
    public:
        //! Constructor.
        //! @param constructParams the set of construction params for the weapon instance
        BaseWeapon(const ConstructParams& constructParams);
        ~BaseWeapon() override = default;
        
        //! IWeapon interface
        //! @{
        WeaponIndex GetWeaponIndex() const override;
        const WeaponParams& GetParams() const override;
        void UpdateWeaponState(WeaponState& weaponState, float deltaTime) override;
        bool CanStartNextEvent(const WeaponState& weaponState, WeaponStatus requiredStatus) const override;
        bool TryStartFire(WeaponState& weaponState, const FireParams& fireParams) override;
        const FireParams& GetFireParams() const override;
        void SetFireParams(const FireParams& fireParams) override;
        const ClientEffect& GetActivateEffect() const override;
        const ClientEffect& GetImpactEffect() const override;
        const ClientEffect& GetDamageEffect() const override;
        int32_t GetAmmoTypeSurfaceIndex() const override;
        //! @}

    protected:

        //! Performs internal activation logic and weapons book keeping.
        //! @param weaponState         the weapon state being updated
        //! @param validateFiringState if true, internal firing state will be checked to validate that the activation event is valid
        //! @return boolean true if the activation was successful
        bool ActivateInternal(WeaponState& weaponState, bool validateFiringState);

        //! Performs internal entity gathering on weapon activation.
        //! @param eventData  specific data regarding the weapon activation
        //! @param outResults reference to the output structure to store gathered entities in
        bool GatherEntities(const ActivateEvent& eventData, IntersectResults& outResults);

        //! Performs internal entity gathering on weapon activation.
        //! @param eventData  specific data regarding the weapon activation
        //! @param outResults reference to the output structure to store gathered entities in
        ShotResult GatherEntitiesMultisegment(float deltaTime, ActiveShot& inOutActiveShot, IntersectResults& outResults);

        //! Dispatches all pending hit callbacks to the weapons listener.
        //! @param gatherResults the structure containing pending hit entities
        //! @param eventData     specific data regarding the weapon activation
        void DispatchHitEvents(const IntersectResults& gatherResults, const ActivateEvent& eventData, const NetEntityIdSet& prefilteredNetEntityIds);

        //! Internally called on every weapon activation.
        //! @param deltaTime              the time in seconds this activate event corresponds to
        //! @param weaponState            the weapons internal state structure
        //! @param weaponOwner            the weapons owning entity
        //! @param eventData              contains details of the activation event
        //! @param dispatchHitEvents      if true, the Activate call will invoke hit events for gathered entities
        //! @param dispatchActivateEvents if true, the Activate call will invoke activate events for valid activations
        //! @param forceSkipGather        if true, skip the gather step of the activation
        virtual void Activate
        (
            float deltaTime,
            WeaponState& weaponState,
            const Multiplayer::ConstNetworkEntityHandle weaponOwner,
            ActivateEvent& eventData,
            bool dispatchHitEvents,
            bool dispatchActivateEvents,
            bool forceSkipGather
        ) = 0;

        //! Ticks the active shots for this weapon.
        //! @param weaponState reference to the predictive state for this weapon
        //! @param deltaTime   the amount of time we are ticking over
        virtual void TickActiveShots(WeaponState& weaponState, float deltaTime) = 0;

        // Do not allow assignment
        BaseWeapon& operator =(const BaseWeapon&) = delete;

        const Multiplayer::ConstNetworkEntityHandle m_owningEntity;
        const WeaponIndex  m_weaponIndex;
        const WeaponParams m_weaponParams;

        WeaponListener& m_weaponListener;
        ClientEffect m_activateEffect;
        ClientEffect m_impactEffect;
        ClientEffect m_damageEffect;
        FireParams   m_fireParams;
        NetEntityIdSet m_gatheredNetEntityIds;

        int32_t m_ammoSurfaceTypeIndex = -1;
    };

    //! Factory function to create an appropriate IWeapon instance given the provided ConstructParams.
    //! @param constructParams parameters to control what type of weapon is instantiated and how its initialized
    //! @return pointer to the IWeapon instance, caller assumes ownership
    AZStd::unique_ptr<IWeapon> CreateWeapon(const ConstructParams& constructParams);
}
