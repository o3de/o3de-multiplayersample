/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <GameplayEffectsNotificationBus.h>
#include <MultiplayerSampleTypes.h>
#include <AzCore/Component/ComponentApplicationBus.h>
#include <AzCore/Component/TransformBus.h>
#include <AzCore/Settings/SettingsRegistry.h>
#include <Source/Components/Multiplayer/EnergyBallComponent.h>
#include <Source/Components/Multiplayer/EnergyCannonComponent.h>
#include <Components/PerfTest/NetworkPrefabSpawnerComponent.h>

namespace MultiplayerSample
{
    void EnergyCannonComponent::Reflect(AZ::ReflectContext* context)
    {
        AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context);
        if (serializeContext)
        {
            serializeContext->Class<EnergyCannonComponent, EnergyCannonComponentBase>()
                ->Version(1);
        }
        EnergyCannonComponentBase::Reflect(context);
    }

    void EnergyCannonComponent::OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        m_effect = GetFiringEffect();
        m_effect.Initialize();
    }

    void EnergyCannonComponent::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
    }

#if AZ_TRAIT_CLIENT
    void EnergyCannonComponent::HandleRPC_TriggerBuildup([[maybe_unused]] AzNetworking::IConnection* invokingConnection)
    {
        m_effect.TriggerEffect(GetEntity()->GetTransform()->GetWorldTM());
    }

    void EnergyCannonComponent::KillBuildupEffect() const
    {
        m_effect.StopEffect();
    }
#endif


    EnergyCannonComponentController::EnergyCannonComponentController(EnergyCannonComponent& parent)
        : EnergyCannonComponentControllerBase(parent)
    {
    }

    void EnergyCannonComponentController::OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
#if AZ_TRAIT_SERVER
        if (GetRateOfFireMs() > AZ::TimeMs{ 0 })
        {
            m_firingEvent.Enqueue(GetRateOfFireMs(), true);
        }
#endif
    }

    void EnergyCannonComponentController::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
#if AZ_TRAIT_SERVER
        m_firingEvent.RemoveFromQueue();
#endif
    }

#if AZ_TRAIT_SERVER
    void EnergyCannonComponentController::OnTriggerBuildup()
    {
        // This RPC starts the buildup effect on the client, we want it to start before the actual ball launch event occurs to make everyhing line up nicely
        RPC_TriggerBuildup();
    }

    void EnergyCannonComponentController::OnFireEnergyBall()
    {
        const AZ::Transform& cannonTm = GetEntity()->GetTransform()->GetWorldTM();
        const AZ::Vector3 effectOffset = GetFiringEffect().GetEffectOffset();
        const AZ::Vector3 ballPosition = cannonTm.TransformPoint(effectOffset);
        const AZ::Vector3 forward = cannonTm.TransformVector(GetFireVector());

        PrefabCallbacks callbacks;
        callbacks.m_onActivateCallback = [this, ballPosition, forward](AZStd::shared_ptr<AzFramework::EntitySpawnTicket> ticket, AzFramework::SpawnableConstEntityContainerView view)
        {
            if (view.empty())
            {
                return;
            }

            const auto ticketId = ticket->GetId();
            for (const AZ::Entity* entity : view)
            {
                if (EnergyBallComponent* ballComponent = entity->FindComponent<EnergyBallComponent>())
                {
                    ballComponent->RPC_LaunchBall(ballPosition, forward, GetNetEntityId());
                    m_triggerBuildupEvent.Enqueue(GetRateOfFireMs() - GetBuildUpTimeMs(), false);
                }
            }

            // Save the spawn ticket, otherwise the prefab will immediately despawn due to the ticket's destruction
            m_spawnedProjectiles.insert(AZStd::make_pair(ticketId, AZStd::move(ticket)));
        };

        GetParent().GetNetworkPrefabSpawnerComponent()->SpawnPrefabAsset
        (
            AZ::Transform::CreateFromQuaternionAndTranslation(AZ::Quaternion::CreateIdentity(), ballPosition),
            GetProjectileSpawnable(),
            AZStd::move(callbacks)
        );
    }
#endif
}
