/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <Source/AutoGen/NetworkWeaponsComponent.AutoComponent.h>
#include <Source/Components/NetworkAiComponent.h>
#include <Source/Weapons/IWeapon.h>
#include <StartingPointInput/InputEventNotificationBus.h>

namespace DebugDraw { class DebugDrawRequests; }

namespace MultiplayerSample
{
    // Input Event Ids for Player Controls
    const StartingPointInput::InputEventNotificationId DrawEventId("drawWeapon");
    const StartingPointInput::InputEventNotificationId FirePrimaryEventId("firePrimaryWeapon");
    const StartingPointInput::InputEventNotificationId FireSecondaryEventId("fireSecondaryWeapon");

    const WeaponIndex PrimaryWeaponIndex   = WeaponIndex{ 0 };
    const WeaponIndex SecondaryWeaponIndex = WeaponIndex{ 1 };

    using OnWeaponActivateEvent = AZ::Event<const WeaponActivationInfo&>;
    using OnWeaponPredictHitEvent = AZ::Event<const WeaponHitInfo&>;
    using OnWeaponConfirmHitEvent = AZ::Event<const WeaponHitInfo&>;

    class NetworkWeaponsComponent
        : public NetworkWeaponsComponentBase
        , private WeaponListener
    {
    public:
        AZ_MULTIPLAYER_COMPONENT(MultiplayerSample::NetworkWeaponsComponent, s_networkWeaponsComponentConcreteUuid, MultiplayerSample::NetworkWeaponsComponentBase);

        static void Reflect(AZ::ReflectContext* context);

        NetworkWeaponsComponent();

        void OnInit() override;
        void OnActivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
        void OnDeactivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;

#if AZ_TRAIT_CLIENT
        void HandleSendConfirmHit(AzNetworking::IConnection* invokingConnection, const WeaponIndex& weaponIndex, const HitEvent& hitEvent) override;
#endif
        void ActivateWeaponWithParams(WeaponIndex weaponIndex, WeaponState& weaponState, const FireParams& fireParams, bool validateActivations);

        IWeapon* GetWeapon(WeaponIndex weaponIndex) const;

        void AddOnWeaponActivateEventHandler(OnWeaponActivateEvent::Handler& handler);
        void AddOnWeaponPredictHitEventHandler(OnWeaponPredictHitEvent::Handler& handler);
        void AddOnWeaponConfirmHitEventHandler(OnWeaponConfirmHitEvent::Handler& handler);

        AZ::Vector3 GetCurrentShotStartPosition();

    private:
        //! WeaponListener interface
        //! @{
        void OnWeaponActivate(const WeaponActivationInfo& activationInfo) override;
        void OnWeaponHit(const WeaponHitInfo& hitInfo) override;
        //! @}

        void OnWeaponPredictHit(const WeaponHitInfo& hitInfo);
        void OnWeaponConfirmHit(const WeaponHitInfo& hitInfo);

        void OnUpdateActivationCounts(int32_t index, uint8_t value);
        void OnTickSimulatedWeapons(float seconds);

        using WeaponPointer = AZStd::unique_ptr<IWeapon>;
        AZStd::array<WeaponPointer, MaxWeaponsPerComponent> m_weapons;

        AZ::Event<int32_t, uint8_t>::Handler m_activationCountHandler;
        AZStd::array<WeaponState, MaxWeaponsPerComponent> m_simulatedWeaponStates;
        AZStd::array<int32_t, MaxWeaponsPerComponent> m_fireBoneJointIds;

        DebugDraw::DebugDrawRequests* m_debugDraw = nullptr;

        OnWeaponActivateEvent m_onWeaponActivateEvent;
        OnWeaponPredictHitEvent m_onWeaponPredictHitEvent;
        OnWeaponConfirmHitEvent m_onWeaponConfirmHitEvent;

        AZ::ScheduledEvent m_tickSimulatedWeapons{[this]()
        {
            OnTickSimulatedWeapons(AZ::TimeMsToSeconds(m_tickSimulatedWeapons.TimeInQueueMs()));
        }, AZ::Name("TickSimulatedWeapons")};
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
        friend class NetworkAiComponentController;

        void UpdateAI();
        bool ShouldProcessInput() const;

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

        AZ::ScheduledEvent m_updateAI;
        NetworkAiComponentController* m_networkAiComponentController = nullptr;

        // Technically these values should never migrate hosts since they are maintained by the autonomous client
        // But due to how the stress test chaos monkey operates, it puppets these values on the server to mimick a client
        // This means these values can and will migrate between hosts (and lose any stored state)
        // We will need to consider moving these values to Authority to Server network properties if the design doesn't change
        bool m_aiEnabled = false;
        bool m_weaponDrawn = true;
        bool m_weaponDrawnChanged = false;
        WeaponActivationBitset m_weaponFiring;
    };
}
