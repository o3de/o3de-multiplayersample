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

    void EnergyBallComponent::HandleRPC_BallExplosion([[maybe_unused]] AzNetworking::IConnection* invokingConnection, const AZ::Vector3& location)
    {
        AZ::Transform transform = AZ::Transform::CreateFromQuaternionAndTranslation(AZ::Quaternion::CreateIdentity(), location);
        m_effect.TriggerEffect(transform);

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

        m_filteredNetEntityIds.clear();
        m_filteredNetEntityIds.insert(owningNetEntityId);
        m_filteredNetEntityIds.insert(GetNetEntityId());
        m_direction = direction;

        // Move the entity to the start position
        GetEntity()->GetTransform()->SetWorldTranslation(startingPosition);

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

        IntersectResults results;
        const ActivateEvent activateEvent{ GetEntity()->GetTransform()->GetWorldTM(), position, m_shooterNetEntityId, GetNetEntityId() };
        GatherEntities(GetGatherParams(), activateEvent, m_filteredNetEntityIds, results);
        if (!results.empty())
        {
            bool shouldTerminate = false;
            for (const IntersectResult& result : results)
            {
                shouldTerminate = true;

                Multiplayer::ConstNetworkEntityHandle handle = Multiplayer::GetMultiplayer()->GetNetworkEntityManager()->GetEntity(result.m_netEntityId);
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

            if (shouldTerminate)
            {
                HideEnergyBall();
            }
        }
    }

    void EnergyBallComponentController::HideEnergyBall()
    {
        RPC_BallExplosion(GetEntity()->GetTransform()->GetWorldTM().GetTranslation());

        Physics::RigidBodyRequestBus::Event(GetEntityId(), &Physics::RigidBodyRequestBus::Events::DisablePhysics);
        Physics::RigidBodyRequestBus::Event(GetEntityId(), &Physics::RigidBodyRequestBus::Events::SetLinearVelocity, AZ::Vector3::CreateZero());

        // move self and increment resetCount to prevent transform interpolation
        AZ::TransformBus::Event(GetEntityId(), &AZ::TransformBus::Events::SetWorldTranslation, AZ::Vector3::CreateAxisZ(-1000.f));
        GetNetworkTransformComponentController()->ModifyResetCount()++;
    }
#endif
}
