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
#include <AzCore/EBus/IEventScheduler.h>
#include <AzFramework/Physics/Components/SimulatedBodyComponentBus.h>
#include <AzFramework/Physics/RigidBodyBus.h>
#include <WeaponNotificationBus.h>

#if AZ_TRAIT_CLIENT
#   include <PopcornFX/PopcornFXBus.h>
#   include <DebugDraw/DebugDrawBus.h>
#endif

namespace MultiplayerSample
{
    AZ_CVAR(float, sv_EnergyBallImpulseScalar, 500.0f, nullptr, AZ::ConsoleFunctorFlags::Null, "A fudge factor for imparting impulses on rigid bodies due to weapon hits");
    AZ_CVAR(bool, cl_EnergyBallDebugDraw, false, nullptr, AZ::ConsoleFunctorFlags::DontReplicate, "When turned on this will draw the current energy ball location");
    AZ_CVAR(float, cl_EnergyBallDebugDrawSeconds, 0.0f, nullptr, AZ::ConsoleFunctorFlags::DontReplicate, "The number of seconds of draw history to preserve for the energy ball");

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
#if AZ_TRAIT_CLIENT
        m_effect = GetExplosionEffect();
        m_effect.Initialize(GameEffect::EmitterType::FireAndForget);

        AZ::EntityBus::Handler::BusConnect(GetEntityId());
        if (cl_EnergyBallDebugDraw)
        {
            m_debugDrawEvent.Enqueue(AZ::TimeMs{ 0 }, true);
        }
#endif
    }

    void EnergyBallComponent::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
#if AZ_TRAIT_CLIENT
        m_effect = {};
        AZ::EntityBus::Handler::BusDisconnect();
        m_debugDrawEvent.RemoveFromQueue();
#endif
    }

#if AZ_TRAIT_CLIENT
    void EnergyBallComponent::OnEntityDeactivated([[maybe_unused]] const AZ::EntityId& entityId)
    {
        // Perform hit / explosion logic when this entity deactivates, but *before* the deactivation sequence is
        // actually running. This allows us to call the WeaponsNotificationBus to notify other components (like Script Canvas)
        // on this entity to perform hit logic. If we waited to run this until OnDeactivate, the other components would no
        // longer be active and wouldn't have a chance to process the logic.

        // Create an explosion effect wherever the ball was last at before deactivating.
        m_effect.TriggerEffect(GetEntity()->GetTransform()->GetWorldTM());

        auto hitEvent = GetHitEvent();

        // Notify this entity about the weapon impact for every entity that was hit, this allows for blast decals.
        for (const HitEntity& hitEntity : hitEvent.m_hitEntities)
        {
            const AZ::Transform hitTransform = AZ::Transform::CreateLookAt(hitEntity.m_hitPosition, hitEntity.m_hitPosition + hitEntity.m_hitNormal, AZ::Transform::Axis::ZPositive);
            const Multiplayer::ConstNetworkEntityHandle handle = Multiplayer::GetNetworkEntityManager()->GetEntity(hitEntity.m_hitNetEntityId);
            const AZ::EntityId hitEntityId = handle.Exists() ? handle.GetEntity()->GetId() : AZ::EntityId();
            WeaponNotificationBus::Broadcast(&WeaponNotificationBus::Events::OnWeaponImpact, GetEntity()->GetId(), hitTransform, hitEntityId);
        }
    }

    void EnergyBallComponent::DebugDraw()
    {
        if (cl_EnergyBallDebugDraw)
        {
            auto* shapeConfig = GetGatherParams().GetCurrentShapeConfiguration();
            if (shapeConfig->GetShapeType() == Physics::ShapeType::Sphere)
            {
                const Physics::SphereShapeConfiguration* sphere = static_cast<const Physics::SphereShapeConfiguration*>(shapeConfig);
                float debugRadius = sphere->m_radius;

                DebugDraw::DebugDrawRequestBus::Broadcast(
                    &DebugDraw::DebugDrawRequestBus::Events::DrawSphereAtLocation,
                    GetEntity()->GetTransform()->GetWorldTM().GetTranslation(),
                    debugRadius,
                    AZ::Colors::Green,
                    cl_EnergyBallDebugDrawSeconds
                );
            }
            else if (shapeConfig->GetShapeType() == Physics::ShapeType::Box)
            {
                const Physics::BoxShapeConfiguration* box = static_cast<const Physics::BoxShapeConfiguration*>(shapeConfig);
                AZ::Obb obb = AZ::Obb::CreateFromPositionRotationAndHalfLengths(
                    GetEntity()->GetTransform()->GetWorldTM().GetTranslation(),
                    GetEntity()->GetTransform()->GetWorldTM().GetRotation(),
                    box->m_dimensions / 2.0f
                );

                DebugDraw::DebugDrawRequestBus::Broadcast(
                    &DebugDraw::DebugDrawRequestBus::Events::DrawObb,
                    obb,
                    AZ::Colors::Green,
                    cl_EnergyBallDebugDrawSeconds
                );
            }
            else if (shapeConfig->GetShapeType() == Physics::ShapeType::Capsule)
            {
                AZ_Error("EnergyBall", false, "Capsule shape type not currently supported with energy ball debug visualization.");
            }
        }
    }
#endif


    EnergyBallComponentController::EnergyBallComponentController(EnergyBallComponent& parent)
        : EnergyBallComponentControllerBase(parent)
    {
    }

    void EnergyBallComponentController::OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
    }

    void EnergyBallComponentController::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
#if AZ_TRAIT_SERVER
        m_collisionCheckEvent.RemoveFromQueue();
        m_killEvent.RemoveFromQueue();
#endif
    }

#if AZ_TRAIT_SERVER
    void EnergyBallComponentController::HandleRPC_LaunchBall(AzNetworking::IConnection* invokingConnection, const AZ::Vector3& startingPosition, const AZ::Vector3& direction, const Multiplayer::NetEntityId& owningNetEntityId)
    {
        AZ_Assert(!m_killEvent.IsScheduled(), "Launching the same ball more than once isn't supported.");

        m_collisionCheckEvent.Enqueue(AZ::TimeMs{ 10 }, true);

        SetVelocity(direction * GetGatherParams().m_travelSpeed);

        m_shooterNetEntityId = owningNetEntityId;
        m_filteredNetEntityIds.clear();
        m_filteredNetEntityIds.insert(owningNetEntityId);
        m_filteredNetEntityIds.insert(GetNetEntityId());
        m_direction = direction;

        // Move the entity to the start position
        GetNetworkTransformComponentController()->HandleMultiplayerTeleport(invokingConnection, startingPosition);

        // We want to sweep our transform during intersect tests to avoid the ball tunneling through targets
        m_lastSweepTransform = GetEntity()->GetTransform()->GetWorldTM();

        // Enqueue our kill event
        m_killEvent.Enqueue(GetLifetimeMs(), false);
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
                ModifyHitEvent().m_hitEntities.emplace_back(hitEntity);

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

            KillEnergyBall();
        }

        // Update our last sweep transform for the next time we check collision
        m_lastSweepTransform = GetEntity()->GetTransform()->GetWorldTM();
    }

    void EnergyBallComponentController::KillEnergyBall()
    {
        m_collisionCheckEvent.RemoveFromQueue();
        m_killEvent.RemoveFromQueue();

        SetVelocity(AZ::Vector3::CreateZero());

        auto& hitEvent = ModifyHitEvent();

        hitEvent.m_target = GetEntity()->GetTransform()->GetWorldTM().GetTranslation();
        hitEvent.m_shooterNetEntityId = m_shooterNetEntityId;
        hitEvent.m_projectileNetEntityId = GetNetEntityId();

        // Immediately remove the entity.
        const Multiplayer::NetEntityId netEntityId = GetNetEntityId();
        const Multiplayer::ConstNetworkEntityHandle entityHandle = Multiplayer::GetNetworkEntityManager()->GetEntity(netEntityId);
        Multiplayer::GetNetworkEntityManager()->MarkForRemoval(entityHandle);
    }
#endif
}
