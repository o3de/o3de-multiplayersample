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

#include <Source/Components/NetworkWeaponsComponent.h>
#include <Source/Components/NetworkAnimationComponent.h>
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

    void NetworkWeaponsComponent::OnActivate([[maybe_unused]] const WeaponActivationInfo& activationInfo)
    {
    }

    void NetworkWeaponsComponent::OnPredictHit([[maybe_unused]] const WeaponHitInfo& hitInfo)
    {
    }

    void NetworkWeaponsComponent::OnConfirmHit([[maybe_unused]] const WeaponHitInfo& hitInfo)
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
        GetNetworkAnimationComponentController()->ModifyActiveAnimStates().SetBit(aznumeric_cast<AZStd::size_t>(CharacterAnimState::Aiming), weaponInput->m_draw);

        for (AZStd::size_t weaponIndex = 0; weaponIndex < MaxWeaponsPerComponent; ++weaponIndex)
        {
            const CharacterAnimState animState = CharacterAnimState::Shooting;
            GetNetworkAnimationComponentController()->ModifyActiveAnimStates().SetBit(aznumeric_cast<AZStd::size_t>(animState), weaponInput->m_firing.GetBit(weaponIndex));
        }
    }

    void NetworkWeaponsComponentController::UpdateWeaponFiring([[maybe_unused]] float deltaTime)
    {
        /*
        WeaponsComponent::NetworkInput* weaponsInput = a_Input->FindInput<WeaponsComponent::NetworkInput>();

        pWeapon->UpdateWeaponState(weaponState, a_DeltaTime);

        if ((weaponState.GetStatus() == EWeaponStatus::Firing) && (weaponState.GetCooldownTime() <= 0.0f))
        {
            AZLOG(NET_TraceWeapons, "Weapon predicted activation event for weapon index %u", iWeaponIndex);

            // Temp hack for weapon firing due to late ebus binding in 1.14
            if (m_FireBoneJointIds[iWeaponIndex] == NetworkAnimationComponent::k_InvalidBoneId)
            {
                const char* fireBoneName = GetFireBoneName(iWeaponIndex).GetString();
                m_FireBoneJointIds[iWeaponIndex] = GetNetworkAnimationComponentPredictable()->GetCommonParent().GetBoneIdByName(fireBoneName);
            }

            NovaNet::Transform fireBoneTransform;
            if (!GetNetworkAnimationComponentPredictable()->GetCommonParent().GetJointTransformById(m_FireBoneJointIds[iWeaponIndex], fireBoneTransform))
            {
                NVLOG_WARN("Failed to get transform for fire bone %s, joint Id %u", GetFireBoneName(iWeaponIndex).GetString(), m_FireBoneJointIds[iWeaponIndex]);
            }

            const FireParams& fireParams = pWeapon->GetFireParams();
            const Vec3        position = fireBoneTransform.m_Position;
            const Quat        orientation = Quat::CreateLookRotation((fireParams.GetTargetPosition() - position).Normalized());
            const Transform   transform = Transform(position, orientation);

            ActivateEvent activateEvent(transform, fireParams.GetTargetPosition(), GetNetId(), INVALID_ENTITY_ID);

            DispatchActivateEvent(weaponsInput, a_DeltaTime, pWeapon, weaponState, activateEvent);
        }
        */
    }


    bool NetworkWeaponsComponentController::TryStartFire()
    {
        /*
        AZLOG(NET_TraceWeapons, "Weapon attempting to fire");

        WeaponState& weaponState = ModifyWeaponStates(*a_Input, a_WeaponIndex);

        if (pWeapon->TryStartFire(weaponState, a_FireParams))
        {
            const uint32_t animBit = static_cast<uint32_t>(pWeapon->GetParams().GetAnimFlag());

            GetNetworkAnimationComponentPredictable()->ModifyActiveAnimStates(*a_Input).SetBit(animBit, true);

            return true;
        }
        */
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
            m_weaponFiring.SetBit(aznumeric_cast<AZStd::size_t>(PrimaryWeaponIndex), true);
        }
        else if (*inputId == FireSecondaryEventId)
        {
            m_weaponFiring.SetBit(aznumeric_cast<AZStd::size_t>(SecondaryWeaponIndex), true);
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
            m_weaponFiring.SetBit(aznumeric_cast<AZStd::size_t>(PrimaryWeaponIndex), false);
        }
        else if (*inputId == FireSecondaryEventId)
        {
            m_weaponFiring.SetBit(aznumeric_cast<AZStd::size_t>(SecondaryWeaponIndex), false);
        }
    }

    void NetworkWeaponsComponentController::OnHeld([[maybe_unused]] float value)
    {
        ;
    }
}
