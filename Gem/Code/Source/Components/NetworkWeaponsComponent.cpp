/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <Source/Components/NetworkWeaponsComponent.h>

#include <Source/Components/NetworkAiComponent.h>
#include <Source/Components/NetworkAnimationComponent.h>
#include <Source/Components/NetworkHealthComponent.h>
#include <Multiplayer/Components/NetworkRigidBodyComponent.h>
#include <Source/Components/NetworkSimplePlayerCameraComponent.h>
#include <Source/Weapons/BaseWeapon.h>
#include <AzCore/Component/TransformBus.h>
#include <AzFramework/Physics/PhysicsScene.h>
#include <AzFramework/Physics/Common/PhysicsSceneQueries.h>
#include <DebugDraw/DebugDrawBus.h>

namespace MultiplayerSample
{
    AZ_CVAR(bool, cl_WeaponsDrawDebug, false, nullptr, AZ::ConsoleFunctorFlags::Null, "If enabled, weapons will debug draw various important events");
    AZ_CVAR(float, cl_WeaponsDrawDebugSize, 0.25f, nullptr, AZ::ConsoleFunctorFlags::Null, "The size of sphere to debug draw during weapon events");
    AZ_CVAR(float, cl_WeaponsDrawDebugDurationSec, 10.0f, nullptr, AZ::ConsoleFunctorFlags::Null, "The number of seconds to display debug draw data");
    AZ_CVAR(float, sv_WeaponsImpulseScalar, 750.0f, nullptr, AZ::ConsoleFunctorFlags::Null, "A fudge factor for imparting impulses on rigid bodies due to weapon hits");
    AZ_CVAR(float, sv_WeaponsStartPositionClampRange, 1.f, nullptr, AZ::ConsoleFunctorFlags::Null, "A fudge factor between the where the client and server say a shot started");
    AZ_CVAR(float, sv_WeaponsDotClamp, 0.35f, nullptr, AZ::ConsoleFunctorFlags::Null, "Acceptable dot product range for a shot between the camera raycast and weapon raycast.");
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

    NetworkWeaponsComponent::NetworkWeaponsComponent()
        : NetworkWeaponsComponentBase()
        , m_activationCountHandler([this](int32_t index, uint8_t value) { OnUpdateActivationCounts(index, value); })
    {
        ;
    }

    void NetworkWeaponsComponent::OnInit()
    {
        AZStd::uninitialized_fill_n(m_fireBoneJointIds.data(), MaxWeaponsPerComponent, InvalidBoneId);
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

        if (IsNetEntityRoleClient())
        {
            ActivationCountsAddEvent(m_activationCountHandler);
        }

        if (m_debugDraw == nullptr)
        {
            m_debugDraw = DebugDraw::DebugDrawRequestBus::FindFirstHandler();
        }
    }

    void NetworkWeaponsComponent::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        ;
    }

    void NetworkWeaponsComponent::HandleSendConfirmHit([[maybe_unused]] AzNetworking::IConnection* invokingConnection, const WeaponIndex& weaponIndex, const HitEvent& hitEvent)
    {
        if (GetWeapon(weaponIndex) == nullptr)
        {
            AZLOG_ERROR("Got confirmed hit for null weapon index");
            return;
        }

        WeaponHitInfo weaponHitInfo(*GetWeapon(weaponIndex), hitEvent);
        OnWeaponConfirmHit(weaponHitInfo);
    }

    void NetworkWeaponsComponent::ActivateWeaponWithParams(WeaponIndex weaponIndex, WeaponState& weaponState, const FireParams& fireParams, bool validateActivations)
    {
        const AZ::Vector3    position = fireParams.m_sourcePosition;
        const AZ::Quaternion orientation = AZ::Quaternion::CreateShortestArc(AZ::Vector3::CreateAxisX(), (fireParams.m_targetPosition - position).GetNormalized());
        const AZ::Transform  transform = AZ::Transform::CreateFromQuaternionAndTranslation(orientation, position);

        ActivateEvent activateEvent{ transform, fireParams.m_targetPosition, GetNetEntityId(), Multiplayer::InvalidNetEntityId };

        IWeapon* weapon = GetWeapon(weaponIndex);
        weapon->Activate(weaponState, GetEntityHandle(), activateEvent, validateActivations);
    }

    IWeapon* NetworkWeaponsComponent::GetWeapon(WeaponIndex weaponIndex) const
    {
        return m_weapons[aznumeric_cast<uint32_t>(weaponIndex)].get();
    }

    void NetworkWeaponsComponent::AddOnWeaponActivateEventHandler(OnWeaponActivateEvent::Handler& handler)
    {
        handler.Connect(m_onWeaponActivateEvent);
    }

    void NetworkWeaponsComponent::AddOnWeaponPredictHitEventHandler(OnWeaponPredictHitEvent::Handler& handler)
    {
        handler.Connect(m_onWeaponPredictHitEvent);
    }

    void NetworkWeaponsComponent::AddOnWeaponConfirmHitEventHandler(OnWeaponConfirmHitEvent::Handler& handler)
    {
        handler.Connect(m_onWeaponConfirmHitEvent);
    }

    void NetworkWeaponsComponent::OnWeaponActivate([[maybe_unused]] const WeaponActivationInfo& activationInfo)
    {
        // If we're replaying inputs then early out
        if (GetNetBindComponent()->IsReprocessingInput())
        {
            return;
        }

        m_onWeaponActivateEvent.Signal(activationInfo);

        if (cl_WeaponsDrawDebug && m_debugDraw)
        {
            m_debugDraw->DrawSphereAtLocation
            (
                activationInfo.m_activateEvent.m_initialTransform.GetTranslation(),
                cl_WeaponsDrawDebugSize,
                AZ::Colors::Green,
                cl_WeaponsDrawDebugDurationSec
            );

            m_debugDraw->DrawSphereAtLocation
            (
                activationInfo.m_activateEvent.m_targetPosition,
                cl_WeaponsDrawDebugSize,
                AZ::Colors::Yellow,
                cl_WeaponsDrawDebugDurationSec
            );
        }
    }

    void NetworkWeaponsComponent::OnWeaponHit(const WeaponHitInfo& hitInfo)
    {
        if (IsNetEntityRoleAuthority())
        {
            OnWeaponConfirmHit(hitInfo);
            static_cast<NetworkWeaponsComponentController*>(GetController())->SendConfirmHit(hitInfo.m_weapon.GetWeaponIndex(), hitInfo.m_hitEvent);
        }
        else
        {
            OnWeaponPredictHit(hitInfo);
        }
    }

    void NetworkWeaponsComponent::OnWeaponPredictHit(const WeaponHitInfo& hitInfo)
    {
        // If we're replaying inputs then early out
        if (GetNetBindComponent()->IsReprocessingInput())
        {
            return;
        }

        m_onWeaponPredictHitEvent.Signal(hitInfo);

        for (uint32_t i = 0; i < hitInfo.m_hitEvent.m_hitEntities.size(); ++i)
        {
            const HitEntity& hitEntity = hitInfo.m_hitEvent.m_hitEntities[i];

            if (cl_WeaponsDrawDebug && m_debugDraw)
            {
                m_debugDraw->DrawSphereAtLocation
                (
                    hitEntity.m_hitPosition,
                    cl_WeaponsDrawDebugSize,
                    AZ::Colors::Orange,
                    cl_WeaponsDrawDebugDurationSec
                );
            }

            AZLOG
            (
                NET_Weapons,
                "Predicted hit on entity %" PRIu64 " at position %f x %f x %f",
                hitEntity.m_hitNetEntityId,
                hitEntity.m_hitPosition.GetX(),
                hitEntity.m_hitPosition.GetY(),
                hitEntity.m_hitPosition.GetZ()
            );
        }
    }

    void NetworkWeaponsComponent::OnWeaponConfirmHit(const WeaponHitInfo& hitInfo)
    {
        if (IsNetEntityRoleAuthority())
        {
            for (const HitEntity& hitEntity : hitInfo.m_hitEvent.m_hitEntities)
            {
                Multiplayer::ConstNetworkEntityHandle entityHandle = Multiplayer::GetMultiplayer()->GetNetworkEntityManager()->GetEntity(hitEntity.m_hitNetEntityId);

                if (entityHandle != nullptr && entityHandle.GetEntity() != nullptr)
                {
                    [[maybe_unused]] const AZ::Vector3& hitCenter = hitInfo.m_hitEvent.m_hitTransform.GetTranslation();
                    [[maybe_unused]] const AZ::Vector3& hitPoint = hitEntity.m_hitPosition;

                    const WeaponParams& weaponParams = hitInfo.m_weapon.GetParams();
                    const HitEffect effect = weaponParams.m_damageEffect;

                    // Presently set to 1 until we capture falloff range
                    float hitDistance = 1.f;
                    float maxDistance = 1.f;
                    float damage = effect.m_hitMagnitude * powf((effect.m_hitFalloff * (1.0f - hitDistance / maxDistance)), effect.m_hitExponent);

                    // Look for physics rigid body component and make impact updates
                    if (Multiplayer::NetworkRigidBodyComponent* rigidBodyComponent = entityHandle.GetEntity()->FindComponent<Multiplayer::NetworkRigidBodyComponent>())
                    {
                        const AZ::Vector3 hitLocation = hitInfo.m_hitEvent.m_hitTransform.GetTranslation();
                        const AZ::Vector3 hitDelta = hitEntity.m_hitPosition - hitLocation;
                        const AZ::Vector3 impulse = hitDelta.GetNormalized() * damage * sv_WeaponsImpulseScalar;
                        rigidBodyComponent->SendApplyImpulse(impulse, hitLocation);
                    }

                    // Look for health component and directly update health based on hit parameters
                    if (NetworkHealthComponent* healthComponent = entityHandle.GetEntity()->FindComponent<NetworkHealthComponent>())
                    {
                        healthComponent->SendHealthDelta(damage * -1.0f);
                    }
                }
            }
        }

        m_onWeaponConfirmHitEvent.Signal(hitInfo);

        // If we're a simulated weapon, or if the weapon is not predictive, then issue material hit effects since the predicted callback above will not get triggered
        [[maybe_unused]] bool shouldIssueMaterialEffects = !HasController() || !hitInfo.m_weapon.GetParams().m_locallyPredicted;

        for (uint32_t i = 0; i < hitInfo.m_hitEvent.m_hitEntities.size(); ++i)
        {
            const HitEntity& hitEntity = hitInfo.m_hitEvent.m_hitEntities[i];

            if (cl_WeaponsDrawDebug && m_debugDraw)
            {
                m_debugDraw->DrawSphereAtLocation
                (
                    hitEntity.m_hitPosition,
                    cl_WeaponsDrawDebugSize,
                    AZ::Colors::Red,
                    cl_WeaponsDrawDebugDurationSec
                );
            }

            AZLOG
            (
                NET_Weapons,
                "Confirmed hit on entity %" PRIu64 " at position %f x %f x %f",
                hitEntity.m_hitNetEntityId,
                hitEntity.m_hitPosition.GetX(),
                hitEntity.m_hitPosition.GetY(),
                hitEntity.m_hitPosition.GetZ()
            );
        }
    }

    void NetworkWeaponsComponent::OnUpdateActivationCounts(int32_t index, uint8_t value)
    {
        IWeapon* weapon = GetWeapon(aznumeric_cast<WeaponIndex>(index));

        if (weapon == nullptr)
        {
            return;
        }

        if (HasController() && weapon->GetParams().m_locallyPredicted)
        {
            // If this is a predicted weapon, exit out because autonomous weapons predict activations
            return;
        }

        AZLOG(NET_Weapons, "Client activation event for weapon index %u", index);

        WeaponState& weaponState = m_simulatedWeaponStates[index];
        const FireParams& fireParams = GetActivationParams(index);
        weapon->SetFireParams(fireParams);

        while (weaponState.m_activationCount != value)
        {
            const bool validateActivations = false;
            ActivateWeaponWithParams(aznumeric_cast<WeaponIndex>(index), weaponState, fireParams, validateActivations);
        }
    }


    NetworkWeaponsComponentController::NetworkWeaponsComponentController(NetworkWeaponsComponent& parent)
        : NetworkWeaponsComponentControllerBase(parent)
        , m_updateAI{[this] { UpdateAI(); }, AZ::Name{ "WeaponsControllerAI" } }
    {
        ;
    }

    void NetworkWeaponsComponentController::OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        NetworkAiComponent* networkAiComponent = GetParent().GetNetworkAiComponent();
        m_aiEnabled = (networkAiComponent != nullptr) ? networkAiComponent->GetEnabled() : false;
        if (m_aiEnabled)
        {
            m_updateAI.Enqueue(AZ::TimeMs{ 0 }, true);
            m_networkAiComponentController = GetNetworkAiComponentController();
        }
        else if (IsNetEntityRoleAutonomous())
        {
            StartingPointInput::InputEventNotificationBus::MultiHandler::BusConnect(DrawEventId);
            StartingPointInput::InputEventNotificationBus::MultiHandler::BusConnect(FirePrimaryEventId);
            StartingPointInput::InputEventNotificationBus::MultiHandler::BusConnect(FireSecondaryEventId);
        }
    }

    void NetworkWeaponsComponentController::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        if (IsNetEntityRoleAutonomous() && !m_aiEnabled)
        {
            StartingPointInput::InputEventNotificationBus::MultiHandler::BusDisconnect(DrawEventId);
            StartingPointInput::InputEventNotificationBus::MultiHandler::BusDisconnect(FirePrimaryEventId);
            StartingPointInput::InputEventNotificationBus::MultiHandler::BusDisconnect(FireSecondaryEventId);
        }
    }

    AZ::Vector3 NetworkWeaponsComponent::GetCurrentShotStartPosition()
    {
        const uint32_t weaponIndexInt = 0;
        const char* fireBoneName = GetFireBoneNames(weaponIndexInt).c_str();
        const int32_t boneIdx = GetNetworkAnimationComponent()->GetBoneIdByName(fireBoneName);

        AZ::Transform fireBoneTransform = AZ::Transform::CreateIdentity();
        if (!GetNetworkAnimationComponent()->GetJointTransformById(boneIdx, fireBoneTransform))
        {
            AZLOG_WARN("Failed to get transform for fire bone joint Id %u", boneIdx);
        }
        return fireBoneTransform.GetTranslation();
    }

    void NetworkWeaponsComponentController::CreateInput(Multiplayer::NetworkInput& input, [[maybe_unused]] float deltaTime)
    {
        // Inputs for your own component always exist
        NetworkWeaponsComponentNetworkInput* weaponInput = input.FindComponentInput<NetworkWeaponsComponentNetworkInput>();

        weaponInput->m_draw = m_weaponDrawn;
        weaponInput->m_firing = m_weaponFiring;

        // All weapon indices point to the same bone so only send one instance
        const uint32_t weaponIndexInt = 0;
        if (weaponInput->m_firing.GetBit(weaponIndexInt))
        {
            weaponInput->m_shotStartPosition = GetParent().GetCurrentShotStartPosition();
        }
    }

    void NetworkWeaponsComponentController::ProcessInput(Multiplayer::NetworkInput& input, [[maybe_unused]] float deltaTime)
    {
        NetworkWeaponsComponentNetworkInput* weaponInput = input.FindComponentInput<NetworkWeaponsComponentNetworkInput>();
        GetNetworkAnimationComponentController()->ModifyActiveAnimStates().SetBit(
            aznumeric_cast<uint32_t>(CharacterAnimState::Aiming), weaponInput->m_draw);

        for (AZStd::size_t weaponIndex = 0; weaponIndex < MaxWeaponsPerComponent; ++weaponIndex)
        {
            const CharacterAnimState animState = CharacterAnimState::Shooting;
            GetNetworkAnimationComponentController()->ModifyActiveAnimStates().SetBit(
                aznumeric_cast<uint32_t>(animState), weaponInput->m_firing.GetBit(static_cast<uint32_t>(weaponIndex)));
        }

        const AZ::Transform worldTm = GetParent().GetEntity()->GetTransform()->GetWorldTM();

        for (uint32_t weaponIndexInt = 0; weaponIndexInt < MaxWeaponsPerComponent; ++weaponIndexInt)
        {
            if (weaponInput->m_firing.GetBit(weaponIndexInt))
            {
                const AZ::Vector3& aimAngles = GetNetworkSimplePlayerCameraComponentController()->GetAimAngles();
                const AZ::Quaternion aimRotation =
                    AZ::Quaternion::CreateRotationZ(aimAngles.GetZ()) * AZ::Quaternion::CreateRotationX(aimAngles.GetX());
                const AZ::Vector3 fwd = AZ::Vector3::CreateAxisY();
                AZ::Vector3 baseCameraOffset;
                AZ::Interface<AZ::IConsole>::Get()->GetCvarValue("cl_cameraOffset", baseCameraOffset);
                const AZ::Vector3 cameraOffset = aimRotation.TransformVector(baseCameraOffset);
                
                const char* fireBoneName = GetFireBoneNames(weaponIndexInt).c_str();
                int32_t boneIdx = GetNetworkAnimationComponentController()->GetParent().GetBoneIdByName(fireBoneName);

                AZ::Transform fireBoneTransform;
                if (!GetNetworkAnimationComponentController()->GetParent().GetJointTransformById(boneIdx, fireBoneTransform))
                {
                    AZLOG_WARN("Failed to get transform for fire bone joint Id %u", boneIdx);
                }

                // Validate the proposed start position is reasonably close to the related bone
                if ((fireBoneTransform.GetTranslation() - weaponInput->m_shotStartPosition).GetLength() > sv_WeaponsStartPositionClampRange)
                {
                    weaponInput->m_shotStartPosition = fireBoneTransform.GetTranslation();
                    AZLOG_WARN("Shot origin was outside of clamp range, resetting to bone position");
                }

                // Setup a default aim target
                const WeaponParams weaponParams = GetWeaponParams(weaponIndexInt);
                AZ::Vector3 aimTarget = worldTm.GetTranslation() + cameraOffset + aimRotation.TransformVector(fwd * weaponParams.m_weaponMaxAimDistance);

                // Cast the ray in the physics system from the center of the camera forward
                if (auto* sceneInterface = AZ::Interface<AzPhysics::SceneInterface>::Get())
                {
                    if (AzPhysics::SceneHandle sceneHandle = sceneInterface->GetSceneHandle(AzPhysics::DefaultPhysicsSceneName);
                        sceneHandle != AzPhysics::InvalidSceneHandle)
                    {
                        AzPhysics::RayCastRequest physicsRayRequest;
                        physicsRayRequest.m_start = worldTm.GetTranslation() + cameraOffset;
                        physicsRayRequest.m_direction = aimRotation.TransformVector(fwd);
                        physicsRayRequest.m_distance = weaponParams.m_weaponMaxAimDistance;
                        physicsRayRequest.m_queryType = AzPhysics::SceneQuery::QueryType::StaticAndDynamic;
                        physicsRayRequest.m_reportMultipleHits = true;

                        AzPhysics::SceneQueryHits result = sceneInterface->QueryScene(sceneHandle, &physicsRayRequest);
                        if (result)
                        {
                            for (AzPhysics::SceneQueryHit hit : result.m_hits)
                            {
                                // Set target to first found intersect within dot tolerance, if any
                                AZ::Vector3 targetDirection = hit.m_position - weaponInput->m_shotStartPosition;
                                AZ::Vector3 aimDirection = physicsRayRequest.m_direction;
                                targetDirection.Normalize();
                                aimDirection.Normalize();
                                if (targetDirection.Dot(aimDirection) > sv_WeaponsDotClamp)
                                {
                                    aimTarget = hit.m_position;
                                    break;
                                }
                            }
                        }
                    }
                }             

                FireParams fireParams{ weaponInput->m_shotStartPosition, aimTarget, Multiplayer::InvalidNetEntityId };
                TryStartFire(aznumeric_cast<WeaponIndex>(weaponIndexInt), fireParams);
            }
        }

        UpdateWeaponFiring(deltaTime);
    }

    void NetworkWeaponsComponentController::UpdateWeaponFiring([[maybe_unused]] float deltaTime)
    {
        for (uint32_t weaponIndexInt = 0; weaponIndexInt < MaxWeaponsPerComponent; ++weaponIndexInt)
        {
            IWeapon* weapon = GetParent().GetWeapon(aznumeric_cast<WeaponIndex>(weaponIndexInt));

            if ((weapon == nullptr) || !weapon->GetParams().m_locallyPredicted)
            {
                continue;
            }

            WeaponState& weaponState = ModifyWeaponStates(weaponIndexInt);
            if ((weaponState.m_status == WeaponStatus::Firing) && (weaponState.m_cooldownTime <= 0.0f))
            {
                AZLOG(NET_Weapons, "Weapon predicted activation event for weapon index %u", weaponIndexInt);

                const bool validateActivations = true;
                const FireParams& fireParams = weapon->GetFireParams();
                GetParent().ActivateWeaponWithParams(
                    aznumeric_cast<WeaponIndex>(weaponIndexInt), weaponState, fireParams, validateActivations);

                if (IsNetEntityRoleAuthority())
                {
                    SetActivationParams(weaponIndexInt, fireParams);
                    SetActivationCounts(weaponIndexInt, weaponState.m_activationCount);
                }
            }
            weapon->UpdateWeaponState(weaponState, deltaTime);
        }
    }

    bool NetworkWeaponsComponentController::TryStartFire(WeaponIndex weaponIndex, const FireParams& fireParams)
    {
        const uint32_t weaponIndexInt = aznumeric_cast<uint32_t>(weaponIndex);
        AZLOG(NET_Weapons, "Weapon start fire on %u", weaponIndexInt);

        IWeapon* weapon = GetParent().GetWeapon(weaponIndex);
        if (weapon == nullptr)
        {
            return false;
        }

        WeaponState& weaponState = ModifyWeaponStates(weaponIndexInt);
        if (weapon->TryStartFire(weaponState, fireParams))
        {
            const uint32_t animBit = static_cast<uint32_t>(weapon->GetParams().m_animFlag);
            if (!GetNetworkAnimationComponentController()->GetActiveAnimStates().GetBit(animBit))
            {
                GetNetworkAnimationComponentController()->ModifyActiveAnimStates().SetBit(animBit, true);
            }
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

    void NetworkWeaponsComponentController::UpdateAI()
    {
        float deltaTime = static_cast<float>(m_updateAI.TimeInQueueMs()) / 1000.f;
        if (m_networkAiComponentController != nullptr)
        {
            m_networkAiComponentController->TickWeapons(*this, deltaTime);
        }
    }
} // namespace MultiplayerSample
