/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <Source/AutoGen/NetworkWeaponsComponent.AutoComponent.h>
#include <Source/Weapons/IWeapon.h>
#include <StartingPointInput/InputEventNotificationBus.h>

namespace MultiplayerSample
{
    // Input Event Ids for Player Controls
    const StartingPointInput::InputEventNotificationId DrawEventId("drawWeapon");
    const StartingPointInput::InputEventNotificationId FirePrimaryEventId("firePrimaryWeapon");
    const StartingPointInput::InputEventNotificationId FireSecondaryEventId("fireSecondaryWeapon");

    const WeaponIndex PrimaryWeaponIndex   = WeaponIndex{ 0 };
    const WeaponIndex SecondaryWeaponIndex = WeaponIndex{ 1 };

    class NetworkWeaponsComponent
        : public NetworkWeaponsComponentBase
        , private WeaponListener
    {
    public:
        AZ_MULTIPLAYER_COMPONENT(MultiplayerSample::NetworkWeaponsComponent, s_networkWeaponsComponentConcreteUuid, MultiplayerSample::NetworkWeaponsComponentBase);

        static void Reflect(AZ::ReflectContext* context);

        void OnInit() override;
        void OnActivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
        void OnDeactivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;

        void HandleSendConfirmHit(AzNetworking::IConnection* invokingConnection, const WeaponIndex& WeaponIndex, const HitEvent& HitEvent) override;
        void HandleSendConfirmProjectileHit(AzNetworking::IConnection* invokingConnection, const WeaponIndex& WeaponIndex, const HitEvent& HitEvent) override;

        IWeapon* GetWeapon(WeaponIndex weaponIndex) const;

    private:
        //! WeaponListener interface
        //! @{
        void OnWeaponActivate(const WeaponActivationInfo& activationInfo) override;
        void OnWeaponPredictHit(const WeaponHitInfo& hitInfo) override;
        void OnWeaponConfirmHit(const WeaponHitInfo& hitInfo) override;
        //! @}

        using WeaponPointer = AZStd::unique_ptr<IWeapon>;
        AZStd::array<WeaponPointer, MaxWeaponsPerComponent> m_weapons;
    };

    class NetworkWeaponsComponentController
        : public NetworkWeaponsComponentControllerBase
        , private StartingPointInput::InputEventNotificationBus::MultiHandler
    {
    public:
        NetworkWeaponsComponentController(NetworkWeaponsComponent& parent);

        void OnActivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
        void OnDeactivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;

        void CreateInput(Multiplayer::NetworkInput& input, float deltaTime) override;
        void ProcessInput(Multiplayer::NetworkInput& input, float deltaTime) override;

    private:

        //! Update pump for player controlled weapons
        //! @param deltaTime the time in seconds since last tick
        void UpdateWeaponFiring(float deltaTime);

        //! Starts a weapon with the frame id from the client
        //! @return boolean true on activate, false if the weapon failed to activate
        virtual bool TryStartFire(WeaponIndex weaponIndex, const FireParams& fireParams);

        //! AZ::InputEventNotificationBus interface
        //! @{
        void OnPressed(float value) override;
        void OnReleased(float value) override;
        void OnHeld(float value) override;
        //! @}

        bool m_weaponDrawn = false;
        WeaponActivationBitset m_weaponFiring;

        AZStd::array<int32_t, MaxWeaponsPerComponent> m_fireBoneJointIds;
    };
}
