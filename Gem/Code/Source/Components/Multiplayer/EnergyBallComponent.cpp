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

#if AZ_TRAIT_CLIENT
        BallActiveAddEvent(m_ballActiveHandler);
#endif
    }

    void EnergyBallComponent::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
#if AZ_TRAIT_CLIENT
        m_ballActiveHandler.Disconnect();
#endif
    }

#if AZ_TRAIT_CLIENT
    void EnergyBallComponent::OnBallActiveChanged(bool active)
    {
        if (active)
        {
            bool startSuccess = false;

            // Set to true to call "Kill" which is deferred, or false to call "Terminate" which is immediate.
            constexpr bool KillOnRestart = true;

            PopcornFX::PopcornFXEmitterComponentRequestBus::EventResult(startSuccess,
                GetEntity()->GetId(), &PopcornFX::PopcornFXEmitterComponentRequestBus::Events::Restart, KillOnRestart);

            AZ_Error("EnergyBall", startSuccess, "Restart call for Energy Ball was unsuccessful.");

            if (cl_EnergyBallDebugDraw)
            {
                m_debugDrawEvent.Enqueue(AZ::TimeMs{ 0 }, true);
            }
        }
        else
        {
            // Crate an explosion effect wherever the ball was last at.
            m_effect.TriggerEffect(GetEntity()->GetTransform()->GetWorldTM());

            bool killSuccess = false;

            // This would ideally use Kill instead of Terminate, but there is a bug in PopcornFX 2.15.4 that if Kill is
            // called on the first tick (which can happen), then the effect will get stuck in a permanent waiting-to-die state,
            // and no amount of Restart calls will ever make it show up again.
            PopcornFX::PopcornFXEmitterComponentRequestBus::EventResult(killSuccess,
                GetEntity()->GetId(), &PopcornFX::PopcornFXEmitterComponentRequestBus::Events::Terminate);

            AZ_Error("EnergyBall", killSuccess, "Kill call for Energy Ball was unsuccessful.");

            m_debugDrawEvent.RemoveFromQueue();
        }
    }

    void EnergyBallComponent::HandleRPC_BallExplosion([[maybe_unused]] AzNetworking::IConnection* invokingConnection, const HitEvent& hitEvent)
    {
        // Create an explosion effect at our current location.
        m_effect.TriggerEffect(GetEntity()->GetTransform()->GetWorldTM());

        // Notify every entity that was hit that they've received a weapon impact, this allows for blast decals.
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
            // Each draw only lasts one frame.
            constexpr float DrawDuration = 0.0f;

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
                    DrawDuration
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
                    DrawDuration
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
#if AZ_TRAIT_SERVER
        SetBallActive(false);
#endif
    }

    void EnergyBallComponentController::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
#if AZ_TRAIT_SERVER
        SetBallActive(false);
#endif
    }

#if AZ_TRAIT_SERVER
    void EnergyBallComponentController::HandleRPC_LaunchBall(AzNetworking::IConnection* invokingConnection, const AZ::Vector3& startingPosition, const AZ::Vector3& direction, const Multiplayer::NetEntityId& owningNetEntityId)
    {
        if (GetBallActive())
        {
            return;
        }

        m_collisionCheckEvent.Enqueue(AZ::TimeMs{ 10 }, true);

        SetBallActive(true);
        SetVelocity(direction * GetGatherParams().m_travelSpeed);

        m_hitEvent.m_hitEntities.clear();

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
        if (!GetBallActive())
        {
            return;
        }

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

            KillEnergyBall();
        }

        // Update our last sweep transform for the next time we check collision
        m_lastSweepTransform = GetEntity()->GetTransform()->GetWorldTM();
    }

    void EnergyBallComponentController::KillEnergyBall()
    {
        if (!GetBallActive())
        {
            return;
        }

        SetBallActive(false);
        m_collisionCheckEvent.RemoveFromQueue();

        m_hitEvent.m_target = GetEntity()->GetTransform()->GetWorldTM().GetTranslation();
        m_hitEvent.m_shooterNetEntityId = m_shooterNetEntityId;
        m_hitEvent.m_projectileNetEntityId = GetNetEntityId();

        RPC_BallExplosion(m_hitEvent);

        // Wait 5 seconds before cleaning up the entity so that the explosion effect has a chance to play out
        // Capture just the netEntityId in case we have a level change or some other operation that clears out entities before our lamda triggers
        const Multiplayer::NetEntityId netEntityId = GetNetEntityId();
        AZ::Interface<AZ::IEventScheduler>::Get()->AddCallback([netEntityId]
            {
                // Fetch the entity handle, ensure its still valid
                const Multiplayer::ConstNetworkEntityHandle entityHandle = Multiplayer::GetNetworkEntityManager()->GetEntity(netEntityId);
                if (entityHandle.Exists())
                {
                    Multiplayer::GetNetworkEntityManager()->MarkForRemoval(entityHandle);
                }
            },
            AZ::Name("Cleanup"),
            AZ::TimeMs{ 5000 }
        );
    }
#endif
}
