/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <Source/Components/NetworkWeaponsComponent.h>
#include <Source/Components/NetworkAnimationComponent.h>
#include <Source/Components/SimplePlayerCameraComponent.h>
#include <Source/Weapons/BaseWeapon.h>

namespace MultiplayerSample
{
    void NetworkWeaponsComponent::NetworkWeaponsComponent::Reflect(AZ::ReflectContext* context)
    {
        AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context);
        if (serializeContext)
        {
            serializeContext->Class<NetworkWeaponsComponent, NetworkWeaponsComponentBase>()
                ->Version(1);
        }
        NetworkWeaponsComponentBase::Reflect(context);
    }

    void NetworkWeaponsComponent::OnInit()
    {
    }

    void NetworkWeaponsComponent::OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        for (uint32_t weaponIndex = 0; weaponIndex < MaxWeaponsPerComponent; ++weaponIndex)
        {
            const ConstructParams constructParams
            {
                GetEntityHandle(),
                aznumeric_cast<WeaponIndex>(weaponIndex),
                GetWeaponParams(weaponIndex),
                *this
            };

            m_weapons[weaponIndex] = AZStd::move(CreateWeapon(constructParams));
        }
    }

    void NetworkWeaponsComponent::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
    }

    void NetworkWeaponsComponent::HandleSendConfirmHit([[maybe_unused]] AzNetworking::IConnection* invokingConnection, [[maybe_unused]] const WeaponIndex& WeaponIndex, [[maybe_unused]] const HitEvent& HitEvent)
    {
    }

    void NetworkWeaponsComponent::HandleSendConfirmProjectileHit([[maybe_unused]] AzNetworking::IConnection* invokingConnection, [[maybe_unused]] const WeaponIndex& WeaponIndex, [[maybe_unused]] const HitEvent& HitEvent)
    {
    }

    IWeapon* NetworkWeaponsComponent::GetWeapon(WeaponIndex weaponIndex) const
    {
        return m_weapons[aznumeric_cast<uint32_t>(weaponIndex)].get();
    }

    void NetworkWeaponsComponent::OnWeaponActivate([[maybe_unused]] const WeaponActivationInfo& activationInfo)
    {
    }

    void NetworkWeaponsComponent::OnWeaponPredictHit([[maybe_unused]] const WeaponHitInfo& hitInfo)
    {
    }

    void NetworkWeaponsComponent::OnWeaponConfirmHit([[maybe_unused]] const WeaponHitInfo& hitInfo)
    {
    }


    NetworkWeaponsComponentController::NetworkWeaponsComponentController(NetworkWeaponsComponent& parent)
        : NetworkWeaponsComponentControllerBase(parent)
    {
    }

    void NetworkWeaponsComponentController::OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        if (IsAutonomous())
        {
            StartingPointInput::InputEventNotificationBus::MultiHandler::BusConnect(DrawEventId);
            StartingPointInput::InputEventNotificationBus::MultiHandler::BusConnect(FirePrimaryEventId);
            StartingPointInput::InputEventNotificationBus::MultiHandler::BusConnect(FireSecondaryEventId);
        }
    }

    void NetworkWeaponsComponentController::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        if (IsAutonomous())
        {
            StartingPointInput::InputEventNotificationBus::MultiHandler::BusDisconnect(DrawEventId);
            StartingPointInput::InputEventNotificationBus::MultiHandler::BusDisconnect(FirePrimaryEventId);
            StartingPointInput::InputEventNotificationBus::MultiHandler::BusDisconnect(FireSecondaryEventId);
        }
    }

    void NetworkWeaponsComponentController::CreateInput(Multiplayer::NetworkInput& input, [[maybe_unused]] float deltaTime)
    {
        // Inputs for your own component always exist
        NetworkWeaponsComponentNetworkInput* weaponInput = input.FindComponentInput<NetworkWeaponsComponentNetworkInput>();

        weaponInput->m_draw = m_weaponDrawn;
        weaponInput->m_firing = m_weaponFiring;
    }

    void NetworkWeaponsComponentController::ProcessInput(Multiplayer::NetworkInput& input, [[maybe_unused]] float deltaTime)
    {
        NetworkWeaponsComponentNetworkInput* weaponInput = input.FindComponentInput<NetworkWeaponsComponentNetworkInput>();
        GetNetworkAnimationComponentController()->ModifyActiveAnimStates().SetBit(aznumeric_cast<uint32_t>(CharacterAnimState::Aiming), weaponInput->m_draw);

        for (AZStd::size_t weaponIndex = 0; weaponIndex < MaxWeaponsPerComponent; ++weaponIndex)
        {
            const CharacterAnimState animState = CharacterAnimState::Shooting;
            GetNetworkAnimationComponentController()->ModifyActiveAnimStates().SetBit(aznumeric_cast<uint32_t>(animState), weaponInput->m_firing.GetBit(static_cast<uint32_t>(weaponIndex)));
        }

        for (AZStd::size_t weaponIndex = 0; weaponIndex < MaxWeaponsPerComponent; ++weaponIndex)
        {
            if (weaponInput->m_firing.GetBit(weaponIndex))
            {
                const AZ::Vector3& aimAngles = GetSimplePlayerCameraComponentController()->GetAimAngles();
                FireParams fireParams{ aimAngles, Multiplayer::InvalidNetEntityId };
                TryStartFire(aznumeric_cast<WeaponIndex>(weaponIndex), fireParams);
            }
        }

        UpdateWeaponFiring(deltaTime);
    }

    void NetworkWeaponsComponentController::UpdateWeaponFiring([[maybe_unused]] float deltaTime)
    {
        for (AZStd::size_t weaponIndex = 0; weaponIndex < MaxWeaponsPerComponent; ++weaponIndex)
        {
            IWeapon* weapon = GetParent().GetWeapon(aznumeric_cast<WeaponIndex>(weaponIndex));

            if ((weapon == nullptr) || !weapon->GetParams().m_locallyPredicted)
            {
                continue;
            }

            WeaponState& weaponState = ModifyWeaponStates(weaponIndex);

            weapon->UpdateWeaponState(weaponState, deltaTime);

            if ((weaponState.m_status == WeaponStatus::Firing) && (weaponState.m_cooldownTime <= 0.0f))
            {
                AZLOG(NET_TraceWeapons, "Weapon predicted activation event for weapon index %u", aznumeric_cast<uint32_t>(weaponIndex));

                // Temp hack for weapon firing due to late ebus binding in 1.14
                if (m_fireBoneJointIds[weaponIndex] == InvalidBoneId)
                {
                    const char* fireBoneName = GetFireBoneNames(weaponIndex).c_str();
                    m_fireBoneJointIds[weaponIndex] = GetNetworkAnimationComponentController()->GetParent().GetBoneIdByName(fireBoneName);
                }

                AZ::Transform fireBoneTransform;
                if (!GetNetworkAnimationComponentController()->GetParent().GetJointTransformById(m_fireBoneJointIds[weaponIndex], fireBoneTransform))
                {
                    AZLOG_WARN("Failed to get transform for fire bone %s, joint Id %u", GetFireBoneNames(weaponIndex).c_str(), m_fireBoneJointIds[weaponIndex]);
                }

                const FireParams&    fireParams = weapon->GetFireParams();
                const AZ::Vector3    position = fireBoneTransform.GetTranslation();
                const AZ::Quaternion orientation = AZ::Quaternion::CreateShortestArc(AZ::Vector3::CreateAxisX(), (fireParams.m_targetPosition - position).GetNormalized());
                const AZ::Transform  transform = AZ::Transform::CreateFromQuaternionAndTranslation(orientation, position);
                ActivateEvent activateEvent{ transform, fireParams.m_targetPosition, GetNetEntityId(), Multiplayer::InvalidNetEntityId };

                //if (cl_DebugDrawWeaponOrigin && (mp_DebugComponentAutonomous != nullptr))
                //{
                //    mp_DebugComponentAutonomous->GetParent().ClientDrawSphere(a_ActivateEvent.GetInitialTransform().m_Position, 0.1f, NovaNet::Vec3(0.0f, 1.0f, 0.0f), 1.0f);
                //    mp_DebugComponentAutonomous->GetParent().ClientDrawSphere(a_ActivateEvent.GetTargetPosition(), 0.5f, NovaNet::Vec3(1.0f, 0.0f, 0.0f), 1.0f);
                //}

                const bool isReplay = GetNetBindComponent()->IsReprocessingInput();
                bool dispatchHitEvents = weapon->GetParams().m_locallyPredicted;
                bool dispatchActivateEvents = weapon->GetParams().m_locallyPredicted;
                bool skipGathers = false;

                if (IsAutonomous())
                {
                    //GetParent().ClearTriggeredPredictedHit();
                }
                else if (IsAuthority())
                {
                    // If client didn't even predict a hit, don't bother checking
                    skipGathers = false;// (a_Input != nullptr) ? !a_Input->m_WeaponGatherRequests.GetBit(a_Weapon->GetWeaponIndex()) : false;
                }

                weapon->Activate(deltaTime, weaponState, GetEntityHandle(), activateEvent, dispatchHitEvents, dispatchActivateEvents, skipGathers);

                if (IsAutonomous())
                {
                    // We predicted a local hit, in this case, let the server know so it can perform a test for this activation
                    //if (GetParent().HasTriggeredPredictedHit())
                    //{
                    //    a_Input->m_WeaponGatherRequests.SetBit(a_Weapon->GetWeaponIndex(), true);
                    //}
                }
                else if (IsAuthority())
                {
                    SetActivationCounts(weaponIndex, weaponState.m_activationCount);
                }
            }
        }
    }

    bool NetworkWeaponsComponentController::TryStartFire(WeaponIndex weaponIndex, const FireParams& fireParams)
    {
        AZLOG(NET_TraceWeapons, "Weapon start fire on %u", aznumeric_cast<uint32_t>(weaponIndex));

        IWeapon* weapon = GetParent().GetWeapon(weaponIndex);

        if (weapon == nullptr)
        {
            return false;
        }

        WeaponState& weaponState = ModifyWeaponStates(aznumeric_cast<uint32_t>(weaponIndex));

        if (weapon->TryStartFire(weaponState, fireParams))
        {
            const uint32_t animBit = static_cast<uint32_t>(weapon->GetParams().m_animFlag);
            GetNetworkAnimationComponentController()->ModifyActiveAnimStates().SetBit(animBit, true);
            return true;
        }

        return false;
    }

    void NetworkWeaponsComponentController::OnPressed([[maybe_unused]] float value)
    {
        const StartingPointInput::InputEventNotificationId* inputId = StartingPointInput::InputEventNotificationBus::GetCurrentBusId();

        if (inputId == nullptr)
        {
            return;
        }
        else if (*inputId == DrawEventId)
        {
            m_weaponDrawn = !m_weaponDrawn;
        }
        else if (*inputId == FirePrimaryEventId)
        {
            m_weaponFiring.SetBit(aznumeric_cast<uint32_t>(PrimaryWeaponIndex), true);
        }
        else if (*inputId == FireSecondaryEventId)
        {
            m_weaponFiring.SetBit(aznumeric_cast<uint32_t>(SecondaryWeaponIndex), true);
        }
    }

    void NetworkWeaponsComponentController::OnReleased([[maybe_unused]] float value)
    {
        const StartingPointInput::InputEventNotificationId* inputId = StartingPointInput::InputEventNotificationBus::GetCurrentBusId();

        if (inputId == nullptr)
        {
            return;
        }
        else if (*inputId == FirePrimaryEventId)
        {
            m_weaponFiring.SetBit(aznumeric_cast<uint32_t>(PrimaryWeaponIndex), false);
        }
        else if (*inputId == FireSecondaryEventId)
        {
            m_weaponFiring.SetBit(aznumeric_cast<uint32_t>(SecondaryWeaponIndex), false);
        }
    }

    void NetworkWeaponsComponentController::OnHeld([[maybe_unused]] float value)
    {
        ;
    }
}
