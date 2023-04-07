/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <Source/Components/Multiplayer/EnergyBallComponent.h>
#include <Source/AutoGen/NetworkHealthComponent.AutoComponent.h>
#include <Multiplayer/Components/NetworkTransformComponent.h>
#include <Multiplayer/Components/NetworkRigidBodyComponent.h>
#include <MultiplayerSampleTypes.h>
#include <AzCore/Component/TransformBus.h>
#include <AzFramework/Physics/RigidBodyBus.h>
#include <WeaponNotificationBus.h>

#if AZ_TRAIT_CLIENT
#   include <PopcornFX/PopcornFXBus.h>
#endif

namespace MultiplayerSample
{
    AZ_CVAR(float, sv_EnergyBallImpulseScalar, 500.0f, nullptr, AZ::ConsoleFunctorFlags::Null, "A fudge factor for imparting impulses on rigid bodies due to weapon hits");

    void EnergyBallComponent::Reflect(AZ::ReflectContext* context)
    {
        AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context);
        if (serializeContext)
        {
            serializeContext->Class<EnergyBallComponent, EnergyBallComponentBase>()
                ->Version(1);
        }
        EnergyBallComponentBase::Reflect(context);
    }

    void EnergyBallComponent::OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        m_effect = GetExplosionEffect();
        m_effect.Initialize();
    }

    void EnergyBallComponent::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
    }

#if AZ_TRAIT_CLIENT
    void EnergyBallComponent::HandleRPC_BallLaunched([[maybe_unused]] AzNetworking::IConnection* invokingConnection, [[maybe_unused]] const AZ::Vector3& location)
    {
        PopcornFX::PopcornFXEmitterComponentRequests* emitterRequests = PopcornFX::PopcornFXEmitterComponentRequestBus::FindFirstHandler(GetEntity()->GetId());
        if (emitterRequests != nullptr)
        {
            emitterRequests->Start();
        }
    }

    void EnergyBallComponent::HandleRPC_BallExplosion([[maybe_unused]] AzNetworking::IConnection* invokingConnection, const HitEvent& hitEvent)
    {
        AZ::Transform transform = AZ::Transform::CreateFromQuaternionAndTranslation(AZ::Quaternion::CreateIdentity(), hitEvent.m_target);
        m_effect.TriggerEffect(transform);

        for (const HitEntity& hitEntity : hitEvent.m_hitEntities)
        {
            const AZ::Transform hitTransform = AZ::Transform::CreateLookAt(hitEntity.m_hitPosition, hitEntity.m_hitPosition + hitEntity.m_hitNormal, AZ::Transform::Axis::ZPositive);
            const Multiplayer::ConstNetworkEntityHandle handle = Multiplayer::GetNetworkEntityManager()->GetEntity(hitEntity.m_hitNetEntityId);
            const AZ::EntityId hitEntityId = handle.Exists() ? handle.GetEntity()->GetId() : AZ::EntityId();
            WeaponNotificationBus::Broadcast(&WeaponNotificationBus::Events::OnWeaponImpact, GetEntity()->GetId(), hitTransform, hitEntityId);
        }

        PopcornFX::PopcornFXEmitterComponentRequests* emitterRequests = PopcornFX::PopcornFXEmitterComponentRequestBus::FindFirstHandler(GetEntity()->GetId());
        if (emitterRequests != nullptr)
        {
            emitterRequests->Kill();
        }
    }
#endif


    EnergyBallComponentController::EnergyBallComponentController(EnergyBallComponent& parent)
        : EnergyBallComponentControllerBase(parent)
    {
    }

    void EnergyBallComponentController::OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
#if AZ_TRAIT_SERVER
        m_collisionCheckEvent.Enqueue(AZ::TimeMs{ 10 }, true);
#endif
    }

    void EnergyBallComponentController::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
#if AZ_TRAIT_SERVER
        m_collisionCheckEvent.RemoveFromQueue();
#endif
    }

#if AZ_TRAIT_SERVER
    void EnergyBallComponentController::HandleRPC_LaunchBall([[maybe_unused]] AzNetworking::IConnection* invokingConnection, const AZ::Vector3& startingPosition, const AZ::Vector3& direction, const Multiplayer::NetEntityId& owningNetEntityId)
    {
        m_shooterNetEntityId = owningNetEntityId;
        m_hitEvent.m_hitEntities.clear();

        m_filteredNetEntityIds.clear();
        m_filteredNetEntityIds.insert(owningNetEntityId);
        m_filteredNetEntityIds.insert(GetNetEntityId());
        m_direction = direction;

        // Move the entity to the start position
        GetEntity()->GetTransform()->SetWorldTranslation(startingPosition);

        // We want to sweep our transform during intersect tests to avoid the ball tunneling through targets
        m_lastSweepTransform = GetEntity()->GetTransform()->GetWorldTM();

        Physics::RigidBodyRequestBus::Event(GetEntityId(), &Physics::RigidBodyRequestBus::Events::EnablePhysics);
        Physics::RigidBodyRequestBus::Event(GetEntityId(), &Physics::RigidBodyRequestBus::Events::SetLinearVelocity, direction * GetGatherParams().m_travelSpeed);

        RPC_BallLaunched(startingPosition);
    }

    void EnergyBallComponentController::HandleRPC_KillBall([[maybe_unused]] AzNetworking::IConnection* invokingConnection)
    {
        HideEnergyBall();
    }

    void EnergyBallComponentController::CheckForCollisions()
    {
        const AZ::Vector3& position = GetEntity()->GetTransform()->GetWorldTM().GetTranslation();
        const HitEffect& effect = GetHitEffect();

        // Sweep from our last checked transform to our current position to avoid tunneling
        const ActivateEvent activateEvent{ m_lastSweepTransform, position, m_shooterNetEntityId, GetNetEntityId() };

        IntersectResults results;
        GatherEntities(GetGatherParams(), activateEvent, m_filteredNetEntityIds, results);

        if (!results.empty())
        {
            for (const IntersectResult& result : results)
            {
                const HitEntity hitEntity{ result.m_position, result.m_normal, result.m_netEntityId };
                m_hitEvent.m_hitEntities.emplace_back(hitEntity);

                const Multiplayer::ConstNetworkEntityHandle handle = Multiplayer::GetNetworkEntityManager()->GetEntity(result.m_netEntityId);
                if (handle.Exists())
                {
                    // Presently set to 1 until we capture falloff range
                    float hitDistance = 1.f;
                    float maxDistance = 1.f;
                    float damage = effect.m_hitMagnitude * powf((effect.m_hitFalloff * (1.0f - hitDistance / maxDistance)), effect.m_hitExponent);

                    // Look for physics rigid body component and make impact updates
                    if (Multiplayer::NetworkRigidBodyComponent* rigidBodyComponent = handle.GetEntity()->FindComponent<Multiplayer::NetworkRigidBodyComponent>())
                    {
                        const AZ::Vector3 explosionCentre = position;
                        const AZ::Vector3 hitObject = handle.GetEntity()->GetTransform()->GetWorldTM().GetTranslation();
                        const AZ::Vector3 impulse = (hitObject - position).GetNormalized() * damage * sv_EnergyBallImpulseScalar;
                        rigidBodyComponent->SendApplyImpulse(impulse, position);
                    }

                    // Look for health component and directly update health based on hit parameters
                    if (NetworkHealthComponent* healthComponent = handle.GetEntity()->FindComponent<NetworkHealthComponent>())
                    {
                        healthComponent->SendHealthDelta(damage * -1.0f);
                    }
                }
            }

            HideEnergyBall();
        }

        // Update our last sweep transform for the next time we check collision
        m_lastSweepTransform = GetEntity()->GetTransform()->GetWorldTM();
    }

    void EnergyBallComponentController::HideEnergyBall()
    {
        m_hitEvent.m_target = GetEntity()->GetTransform()->GetWorldTM().GetTranslation();
        m_hitEvent.m_shooterNetEntityId = m_shooterNetEntityId;
        m_hitEvent.m_projectileNetEntityId = GetNetEntityId();
        RPC_BallExplosion(m_hitEvent);

        Physics::RigidBodyRequestBus::Event(GetEntityId(), &Physics::RigidBodyRequestBus::Events::DisablePhysics);
        Physics::RigidBodyRequestBus::Event(GetEntityId(), &Physics::RigidBodyRequestBus::Events::SetLinearVelocity, AZ::Vector3::CreateZero());

        // move self and increment resetCount to prevent transform interpolation
        AZ::TransformBus::Event(GetEntityId(), &AZ::TransformBus::Events::SetWorldTranslation, AZ::Vector3::CreateAxisZ(-1000.f));
        GetNetworkTransformComponentController()->ModifyResetCount()++;
    }
#endif
}
