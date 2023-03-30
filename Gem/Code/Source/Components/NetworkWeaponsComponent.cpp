/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <Source/Components/NetworkWeaponsComponent.h>
#include <Source/Components/NetworkPlayerMovementComponent.h>
#include <Source/Components/NetworkAiComponent.h>
#include <Source/Components/NetworkAnimationComponent.h>
#include <Source/Components/NetworkHealthComponent.h>
#include <Multiplayer/Components/NetworkRigidBodyComponent.h>
#include <Source/Components/NetworkMatchComponent.h>
#include <Source/Components/NetworkSimplePlayerCameraComponent.h>
#include <Source/Components/Multiplayer/PlayerIdentityComponent.h>
#include <Source/Weapons/BaseWeapon.h>
#include <AzCore/Component/TransformBus.h>
#include <AzCore/Math/Plane.h>
#include <AzFramework/Physics/PhysicsScene.h>
#include <AzFramework/Physics/Common/PhysicsSceneQueries.h>
#include <WeaponNotificationBus.h>

#if AZ_TRAIT_CLIENT
#   include <DebugDraw/DebugDrawBus.h>
#endif

namespace MultiplayerSample
{
    AZ_CVAR(bool, cl_WeaponsDrawDebug, false, nullptr, AZ::ConsoleFunctorFlags::Null, "If enabled, weapons will debug draw various important events");
    AZ_CVAR(float, cl_WeaponsDrawDebugSize, 0.25f, nullptr, AZ::ConsoleFunctorFlags::Null, "The size of sphere to debug draw during weapon events");
    AZ_CVAR(float, cl_WeaponsDrawDebugDurationSec, 10.0f, nullptr, AZ::ConsoleFunctorFlags::Null, "The number of seconds to display debug draw data");
    AZ_CVAR(float, sv_WeaponsImpulseScalar, 750.0f, nullptr, AZ::ConsoleFunctorFlags::Null, "A fudge factor for imparting impulses on rigid bodies due to weapon hits");
    AZ_CVAR(float, sv_WeaponsStartPositionClampRange, 1.f, nullptr, AZ::ConsoleFunctorFlags::Null, "A fudge factor between the where the client and server say a shot started");
    AZ_CVAR(float, sv_WeaponsDotClamp, 0.35f, nullptr, AZ::ConsoleFunctorFlags::Null, "Acceptable dot product range for a shot between the camera raycast and weapon raycast.");

    class BehaviorWeaponNotificationBusHandler
        : public WeaponNotificationBus::Handler
        , public AZ::BehaviorEBusHandler
    {
    public:
        AZ_EBUS_BEHAVIOR_BINDER(BehaviorWeaponNotificationBusHandler, "{8F083B95-4519-4A24-8824-ED5ADA8FC52E}", AZ::SystemAllocator,
            OnWeaponActivate,
            OnWeaponImpact,
            OnWeaponDamage,
            OnConfirmedHitPlayer);

        void OnWeaponActivate(const AZ::Transform& transform) override
        {
            Call(FN_OnWeaponActivate, transform);
        }

        void OnWeaponImpact(const AZ::Transform& transform) override
        {
            Call(FN_OnWeaponImpact, transform);
        }

        void OnWeaponDamage(const AZ::Transform& transform) override
        {
            Call(FN_OnWeaponDamage, transform);
        }

        void OnConfirmedHitPlayer(AZ::EntityId byPlayerEntity, AZ::EntityId otherPlayerEntity) override
        {
            Call(FN_OnConfirmedHitPlayer, byPlayerEntity, otherPlayerEntity);
        }
    };

    void NetworkWeaponsComponent::Reflect(AZ::ReflectContext* context)
    {
        AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context);
        if (serializeContext)
        {
            serializeContext->Class<NetworkWeaponsComponent, NetworkWeaponsComponentBase>()
                ->Version(1);
        }
        NetworkWeaponsComponentBase::Reflect(context);

        AZ::BehaviorContext* behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context);
        if (behaviorContext)
        {
            behaviorContext->EBus<WeaponNotificationBus>("WeaponNotificationBus")
                ->Handler<BehaviorWeaponNotificationBusHandler>();
        }
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

        m_tickSimulatedWeapons.Enqueue(AZ::Time::ZeroTimeMs);

#if AZ_TRAIT_CLIENT
        if (m_debugDraw == nullptr)
        {
            m_debugDraw = DebugDraw::DebugDrawRequestBus::FindFirstHandler();
        }
#endif
    }

    void NetworkWeaponsComponent::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        m_tickSimulatedWeapons.RemoveFromQueue();
    }

#if AZ_TRAIT_CLIENT
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
#endif

    void NetworkWeaponsComponent::ActivateWeaponWithParams(WeaponIndex weaponIndex, WeaponState& weaponState, const FireParams& fireParams, bool validateActivations)
    {
        const AZ::Transform transform = AZ::Transform::CreateLookAt(fireParams.m_sourcePosition, fireParams.m_targetPosition);
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
        WeaponNotificationBus::Broadcast(&WeaponNotificationBus::Events::OnWeaponActivate, activationInfo.m_activateEvent.m_initialTransform);

#if AZ_TRAIT_CLIENT
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

        activationInfo.m_weapon.ExecuteActivateEffect(activationInfo.m_activateEvent.m_initialTransform, activationInfo.m_activateEvent.m_targetPosition);
#endif
    }

    void NetworkWeaponsComponent::OnWeaponHit(const WeaponHitInfo& hitInfo)
    {
        if (IsNetEntityRoleAuthority())
        {
#if AZ_TRAIT_SERVER
            OnWeaponConfirmHit(hitInfo);
            static_cast<NetworkWeaponsComponentController*>(GetController())->SendConfirmHit(hitInfo.m_weapon.GetWeaponIndex(), hitInfo.m_hitEvent);
#endif
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
        WeaponNotificationBus::Broadcast(&WeaponNotificationBus::Events::OnWeaponImpact, hitInfo.m_hitEvent.m_hitTransform);

        for (const auto& hitEntity : hitInfo.m_hitEvent.m_hitEntities)
        {
#if AZ_TRAIT_CLIENT
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

            hitInfo.m_weapon.ExecuteImpactEffect(hitInfo.m_weapon.GetFireParams().m_sourcePosition, hitEntity.m_hitPosition);
#endif

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
#if AZ_TRAIT_SERVER
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
                        // IMPORTANT this impulse is for directional/traced hits only, not suitable for explosions
                        const AZ::Vector3 impulse = hitInfo.m_hitEvent.m_hitTransform.GetBasisY() * damage * sv_WeaponsImpulseScalar;
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
#endif

        m_onWeaponConfirmHitEvent.Signal(hitInfo);
        WeaponNotificationBus::Broadcast(&WeaponNotificationBus::Events::OnWeaponDamage, hitInfo.m_hitEvent.m_hitTransform);

        // If we're a simulated weapon, or if the weapon is not predictive, then issue material hit effects since the predicted callback above will not get triggered
        // Note that materialfx are not hooked up currently as the engine currently doesn't support them
        [[maybe_unused]] bool shouldIssueMaterialEffects = !HasController() || !hitInfo.m_weapon.GetParams().m_locallyPredicted;

        for (const auto& hitEntity : hitInfo.m_hitEvent.m_hitEntities)
        {
#if AZ_TRAIT_CLIENT
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

            Multiplayer::ConstNetworkEntityHandle handle = Multiplayer::GetMultiplayer()->GetNetworkEntityManager()->GetEntity(
                hitEntity.m_hitNetEntityId);
            if (handle.Exists())
            {
                if (const AZ::Entity* entity = handle.GetEntity())
                {
                    if (entity->FindComponent<PlayerIdentityComponent>())
                    {
                        // Hit confirmed on a player
                        WeaponNotificationBus::Broadcast(&WeaponNotificationBus::Events::OnConfirmedHitPlayer, GetEntityId(), entity->GetId());
                    }
                }
            }

            hitInfo.m_weapon.ExecuteDamageEffect(hitInfo.m_weapon.GetFireParams().m_sourcePosition, hitEntity.m_hitPosition);
#endif

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
            constexpr bool validateActivations = false;
            ActivateWeaponWithParams(aznumeric_cast<WeaponIndex>(index), weaponState, fireParams, validateActivations);
        }
    }

    void NetworkWeaponsComponent::OnTickSimulatedWeapons(float seconds)
    {
        for (int weaponIndex = 0; weaponIndex < m_simulatedWeaponStates.size(); ++weaponIndex)
        {
            if (auto* weapon = GetWeapon(static_cast<WeaponIndex>(weaponIndex)))
            {
                weapon->TickActiveShots(m_simulatedWeaponStates[weaponIndex], seconds);
            }
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
        const NetworkAiComponent* networkAiComponent = GetParent().GetNetworkAiComponent();
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
        constexpr uint32_t weaponIndexInt = 0;
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
        INetworkMatch* networkMatchComponent = AZ::Interface<INetworkMatch>::Get();
        if (!networkMatchComponent || (networkMatchComponent->PlayerActionsAllowed() != AllowedPlayerActions::All))
        {
            m_weaponDrawn = false;
            m_weaponFiring = false;
            return;
        }

        // Inputs for your own component always exist
        NetworkWeaponsComponentNetworkInput* weaponInput = input.FindComponentInput<NetworkWeaponsComponentNetworkInput>();

        // Use simple debounce for weapon draw to prevent multiple inputs between frames 
        if (m_weaponDrawnChanged || (m_weaponFiring.AnySet() && !m_weaponDrawn))
        {
            m_weaponDrawn = !m_weaponDrawn;
            m_weaponDrawnChanged = false;
        }

        weaponInput->m_firing = m_weaponFiring;

        // All weapon indices point to the same bone so only send one instance
        if (weaponInput->m_firing.AnySet())
        {
            // Draw weapon automatically if firing
            m_weaponDrawn = true;
            weaponInput->m_shotStartPosition = GetParent().GetCurrentShotStartPosition();
        }

        weaponInput->m_draw = m_weaponDrawn;
    }

    void NetworkWeaponsComponentController::ProcessInput(Multiplayer::NetworkInput& input, [[maybe_unused]] float deltaTime)
    {
        NetworkWeaponsComponentNetworkInput* weaponInput = input.FindComponentInput<NetworkWeaponsComponentNetworkInput>();
        NetworkPlayerMovementComponentNetworkInput* playerInput = input.FindComponentInput<NetworkPlayerMovementComponentNetworkInput>();

        // Enable aiming if our weapon drawn flag is raised
        GetNetworkAnimationComponentController()->ModifyActiveAnimStates().SetBit(aznumeric_cast<uint32_t>(CharacterAnimState::Aiming), weaponInput->m_draw);

        if ((playerInput != nullptr) && playerInput->m_sprint)
        {
            // Stop aiming whenever we're sprinting
            GetNetworkAnimationComponentController()->ModifyActiveAnimStates().SetBit(aznumeric_cast<uint32_t>(CharacterAnimState::Aiming), false);
        }

        // Always disable the recoil shooting flag when we start ProcessInput
        // It will raise again if we actually enter an activation scenario for any weapon
        if (!weaponInput->m_firing.AnySet())
        {
            GetNetworkAnimationComponentController()->ModifyActiveAnimStates().SetBit(aznumeric_cast<uint32_t>(CharacterAnimState::Shooting), false);
        }

        const AZ::Transform cameraTransform = GetNetworkSimplePlayerCameraComponentController()->GetCameraTransform(/*collisionEnabled=*/false);

        for (uint32_t weaponIndexInt = 0; weaponIndexInt < MaxWeaponsPerComponent; ++weaponIndexInt)
        {
            if (weaponInput->m_firing.GetBit(weaponIndexInt))
            {
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
                AZ::Vector3 aimTarget = cameraTransform.GetTranslation() + cameraTransform.GetBasisY() * weaponParams.m_weaponMaxAimDistance;

                // Given a plane centered on the shot start position with the orientation of the camera
                // find the intersection of the camera ray with this plane and use it as the 
                // start position for the trace to avoid any hits behind the weapon 
                const AZ::Plane weaponPlane = AZ::Plane::CreateFromNormalAndPoint(cameraTransform.GetBasisY(), weaponInput->m_shotStartPosition);
                AZ::Vector3 rayStart = cameraTransform.GetTranslation();
                // on success, rayStart will contain the intersection point, on false we'll fallback to the camera translation
                if (!weaponPlane.CastRay(cameraTransform.GetTranslation(), cameraTransform.GetBasisY(), rayStart))
                {
                    AZLOG_WARN("Falling back to detect aim target based on camera origin");
                }

                // Cast the ray in the physics system from the center of the camera forward
                if (auto* sceneInterface = AZ::Interface<AzPhysics::SceneInterface>::Get())
                {
                    if (AzPhysics::SceneHandle sceneHandle = sceneInterface->GetSceneHandle(AzPhysics::DefaultPhysicsSceneName);
                        sceneHandle != AzPhysics::InvalidSceneHandle)
                    {
                        AzPhysics::RayCastRequest physicsRayRequest;
                        physicsRayRequest.m_start = rayStart;
                        physicsRayRequest.m_direction = cameraTransform.GetBasisY();
                        physicsRayRequest.m_distance = weaponParams.m_weaponMaxAimDistance;
                        physicsRayRequest.m_queryType = AzPhysics::SceneQuery::QueryType::StaticAndDynamic;
                        physicsRayRequest.m_reportMultipleHits = true;

                        if (AzPhysics::SceneQueryHits result = sceneInterface->QueryScene(sceneHandle, &physicsRayRequest))
                        {
                            float minDistance = AZStd::numeric_limits<float>::max();
                            for (const AzPhysics::SceneQueryHit& hit : result.m_hits)
                            {
                                // Set target to closest found intersection within dot tolerance, if any
                                AZ::Vector3 targetDirection = hit.m_position - weaponInput->m_shotStartPosition;
                                AZ::Vector3 aimDirection = physicsRayRequest.m_direction;
                                targetDirection.Normalize();
                                aimDirection.Normalize();
                                if ((targetDirection.Dot(aimDirection) > sv_WeaponsDotClamp) && (hit.m_distance <= minDistance))
                                {
                                    aimTarget = hit.m_position;
                                    minDistance = hit.m_distance;
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

#if AZ_TRAIT_SERVER
                if (IsNetEntityRoleAuthority())
                {
                    SetActivationParams(weaponIndexInt, fireParams);
                    SetActivationCounts(weaponIndexInt, weaponState.m_activationCount);
                }
#endif
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

        const uint32_t animBit = static_cast<uint32_t>(weapon->GetParams().m_animFlag);
        WeaponState& weaponState = ModifyWeaponStates(weaponIndexInt);
        if (weapon->TryStartFire(weaponState, fireParams))
        {
            if (!GetNetworkAnimationComponentController()->GetActiveAnimStates().GetBit(animBit))
            {
                GetNetworkAnimationComponentController()->ModifyActiveAnimStates().SetBit(animBit, true);
            }
            return true;
        }
        else if (GetNetworkAnimationComponentController()->GetActiveAnimStates().GetBit(animBit))
        {
            GetNetworkAnimationComponentController()->ModifyActiveAnimStates().SetBit(animBit, false);
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
            m_weaponDrawnChanged = true;
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
#if AZ_TRAIT_SERVER
        float deltaTime = static_cast<float>(m_updateAI.TimeInQueueMs()) / 1000.f;
        if (m_networkAiComponentController != nullptr)
        {
            m_networkAiComponentController->TickWeapons(*this, deltaTime);
        }
#endif
    }
} // namespace MultiplayerSample
