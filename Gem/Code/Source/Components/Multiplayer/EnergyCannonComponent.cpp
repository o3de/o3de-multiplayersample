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
        // Re-using the same ball entity.
        AZ::Entity* ball = nullptr;
        AZ::ComponentApplicationBus::BroadcastResult(ball, &AZ::ComponentApplicationBus::Events::FindEntity, GetEnergyBallEntity());
        if (ball)
        {
            if (EnergyBallComponent* ballComponent = ball->FindComponent<EnergyBallComponent>())
            {
                const AZ::Transform& cannonTm = GetEntity()->GetTransform()->GetWorldTM();
                const AZ::Vector3 forward = cannonTm.TransformVector(AZ::Vector3::CreateAxisY(-1.f));
                const AZ::Vector3 effectOffset = GetFiringEffect().GetEffectOffset();
                ballComponent->RPC_LaunchBall(cannonTm.GetTranslation() + cannonTm.TransformVector(effectOffset), forward, GetNetEntityId());

                // Enqueue our ball kill event
                m_killEvent.Enqueue(GetBallLifetimeMs(), false);
                m_triggerBuildupEvent.Enqueue(GetRateOfFireMs() - GetBuildUpTimeMs(), false);
            }
        }
    }

    void EnergyCannonComponentController::OnKillEnergyBall()
    {
        // Re-using the same ball entity.
        AZ::Entity* ball = nullptr;
        AZ::ComponentApplicationBus::BroadcastResult(ball, &AZ::ComponentApplicationBus::Events::FindEntity, GetEnergyBallEntity());
        if (ball)
        {
            if (EnergyBallComponent* ballComponent = ball->FindComponent<EnergyBallComponent>())
            {
                ballComponent->RPC_KillBall();
            }
        }
    }
#endif
}
