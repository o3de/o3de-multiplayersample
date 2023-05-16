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
#if AZ_TRAIT_CLIENT
        m_effect = {};
#endif
    }

#if AZ_TRAIT_CLIENT
    void EnergyCannonComponent::HandleRPC_TriggerBuildup([[maybe_unused]] AzNetworking::IConnection* invokingConnection)
    {
        m_effect.TriggerEffect(GetEntity()->GetTransform()->GetWorldTM());
    }

    void EnergyCannonComponent::HandleRPC_StopBuildup([[maybe_unused]] AzNetworking::IConnection* invokingConnection)
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
        m_triggerBuildupEvent.RemoveFromQueue();
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
        RPC_StopBuildup();

        const AZ::Transform& cannonTm = GetEntity()->GetTransform()->GetWorldTM();
        const AZ::Vector3 effectOffset = GetFiringEffect().GetEffectOffset();
        const AZ::Vector3 ballPosition = cannonTm.TransformPoint(effectOffset);
        const AZ::Vector3 forward = cannonTm.TransformVector(GetFireVector());

        const Multiplayer::PrefabEntityId prefabEntityId(AZ::Name(GetProjectileSpawnable().m_spawnableAsset.GetHint().c_str()));

        const AZ::Transform transform = AZ::Transform::CreateFromQuaternionAndTranslation(AZ::Quaternion::CreateIdentity(), ballPosition);

        Multiplayer::INetworkEntityManager::EntityList entityList =
            Multiplayer::GetNetworkEntityManager()->CreateEntitiesImmediate(prefabEntityId, Multiplayer::NetEntityRole::Authority, transform);

        Multiplayer::NetworkEntityHandle spawnedEntity;
        if (entityList.size() == 1)
        {
            spawnedEntity = entityList[0];
        }
        else
        {
            AZLOG_WARN("Attempt to spawn prefab %s failed. Check that prefab is network enabled and only contains a single entity. "
                "If multiple entities are in the prefab, only the first one will get deleted. Spawn count: %zu", 
                prefabEntityId.m_prefabName.GetCStr(), entityList.size());
        }

        if (EnergyBallComponent* ballComponent = spawnedEntity.FindComponent<EnergyBallComponent>())
        {
            ballComponent->RPC_LaunchBall(ballPosition, forward, GetNetEntityId());
            m_triggerBuildupEvent.Enqueue(GetRateOfFireMs() - GetBuildUpTimeMs(), false);
        }
    }
#endif
}
