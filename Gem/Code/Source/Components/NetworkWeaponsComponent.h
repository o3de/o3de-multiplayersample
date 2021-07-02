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

#include <Source/AutoGen/NetworkWeaponsComponent.AutoComponent.h>
#include <Source/Weapons/IWeapon.h>
#include <StartingPointInput/InputEventNotificationBus.h>

namespace MultiplayerSample
{
    // Input Event Ids for Player Controls
    const StartingPointInput::InputEventNotificationId DrawEventId("draw");
    const StartingPointInput::InputEventNotificationId FirePrimaryEventId("firePrimary");
    const StartingPointInput::InputEventNotificationId FireSecondaryEventId("fireSecondary");

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

    private:
        //! WeaponListener interface
        //! @{
        void OnActivate(const WeaponActivationInfo& activationInfo) override;
        void OnPredictHit(const WeaponHitInfo& hitInfo) override;
        void OnConfirmHit(const WeaponHitInfo& hitInfo) override;
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
        virtual bool TryStartFire();

        //! AZ::InputEventNotificationBus interface
        //! @{
        void OnPressed(float value) override;
        void OnReleased(float value) override;
        void OnHeld(float value) override;
        //! @}

        bool m_weaponDrawn = false;
        WeaponActivationBitset m_weaponFiring;
    };
}
