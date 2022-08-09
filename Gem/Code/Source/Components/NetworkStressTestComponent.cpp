/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of
 * this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <Source/Components/NetworkStressTestComponent.h>

#include <Source/Components/NetworkAiComponent.h>
#include <Source/Components/NetworkPlayerMovementComponent.h>

#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <Multiplayer/IMultiplayer.h>
#include <Multiplayer/Components/NetBindComponent.h>
#include <Multiplayer/ConnectionData/IConnectionData.h>
#include <Multiplayer/ReplicationWindows/IReplicationWindow.h>

namespace MultiplayerSample
{
    void NetworkStressTestComponent::Reflect(AZ::ReflectContext* context)
    {
        AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context);
        if (serializeContext)
        {
            serializeContext->Class<NetworkStressTestComponent, NetworkStressTestComponentBase>()
                ->Version(1);
        }

        NetworkStressTestComponentBase::Reflect(context);
    }

    void NetworkStressTestComponent::OnInit()
    {
    }

    NetworkStressTestComponentController::NetworkStressTestComponentController(NetworkStressTestComponent& owner)
        : NetworkStressTestComponentControllerBase(owner)
        , m_autoSpawnTimer([this]() { HandleSpawnAiEntity(); }, AZ::Name("StressTestSpawner Event"))
    {
        ;
    }

    void NetworkStressTestComponentController::OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
#ifdef IMGUI_ENABLED
        ImGui::ImGuiUpdateListenerBus::Handler::BusConnect();
#endif
        auto agentType = AZ::Interface<Multiplayer::IMultiplayer>::Get()->GetAgentType();
        switch (agentType)
        {
        case Multiplayer::MultiplayerAgentType::DedicatedServer:
        case Multiplayer::MultiplayerAgentType::ClientServer:
#ifdef IMGUI_ENABLED
            m_isServer = true;
#endif
            break;
        default:
            break;
        }

        if (GetAutoSpawnIntervalMs() > AZ::Time::ZeroTimeMs)
        {
            m_autoSpawnTimer.Enqueue(GetAutoSpawnIntervalMs(), true);
        }
    }

    void NetworkStressTestComponentController::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
#ifdef IMGUI_ENABLED
        ImGui::ImGuiUpdateListenerBus::Handler::BusDisconnect();
#endif
    }

    void NetworkStressTestComponentController::HandleSpawnAiEntity()
    {
        const uint64_t seed = m_seed == 0 ? static_cast<uint64_t>(AZ::Interface<AZ::ITime>::Get()->GetElapsedTimeMs()) : m_seed;
        HandleSpawnAIEntity(nullptr, m_fireIntervalMinMs, m_fireIntervalMaxMs, m_actionIntervalMinMs, m_actionIntervalMaxMs, seed, m_teamID);
    }

#if defined(IMGUI_ENABLED)
    void NetworkStressTestComponentController::OnImGuiMainMenuUpdate()
    {
        if (ImGui::BeginMenu("Multiplayer Sample"))
        {
            ImGui::Checkbox("Entity Spawner", &m_displayEntitySpawner);
            ImGui::EndMenu();
        }
    }

    void NetworkStressTestComponentController::OnImGuiUpdate()
    {
        if (m_displayEntitySpawner)
        {
            if (ImGui::Begin("Entity Spawner", &m_displayEntitySpawner, ImGuiWindowFlags_AlwaysAutoResize))
            {
                DrawEntitySpawner();
            }
        }
    }

    void NetworkStressTestComponentController::DrawEntitySpawner()
    {
        ImGui::SliderInt("Quantity", &m_quantity, 1, 100);
        ImGui::SliderInt("Team ID", &m_teamID, 0, 3);
        ImGui::InputFloat("Fire Interval Min (ms)", &m_fireIntervalMinMs, 0.f, 100000.f);
        ImGui::InputFloat("Fire Interval Max (ms)", &m_fireIntervalMaxMs, 0.f, 100000.f);
        ImGui::InputFloat("Action Interval Min (ms)", &m_actionIntervalMinMs, 0.f, 100000.f);
        ImGui::InputFloat("Action Interval Max (ms)", &m_actionIntervalMaxMs, 0.f, 100000.f);
        constexpr static uint64_t SeedMin = 0;
        constexpr static uint64_t SeedMax = AZStd::numeric_limits<uint64_t>::max();
        ImGui::InputScalar("Seed", ImGuiDataType_U64, &m_seed, &SeedMin, &SeedMax, "%llu");

        if (ImGui::Button("Spawn AI Entity"))
        {
            uint64_t seed = m_seed == 0 ? static_cast<uint64_t>(AZ::Interface<AZ::ITime>::Get()->GetElapsedTimeMs()) : m_seed;

            for (int i = 0; i != m_quantity; ++i)
            {
                if (m_isServer)
                {
                    HandleSpawnAIEntity(
                        nullptr,
                        m_fireIntervalMinMs,
                        m_fireIntervalMaxMs,
                        m_actionIntervalMinMs,
                        m_actionIntervalMaxMs,
                        seed + i,
                        m_teamID);
                }
                else
                {
                    SpawnAIEntity(
                        m_fireIntervalMinMs,
                        m_fireIntervalMaxMs,
                        m_actionIntervalMinMs,
                        m_actionIntervalMaxMs,
                        seed + i,
                        m_teamID);
                }
            }
        }
    }
#endif // defined(IMGUI_ENABLED)

    void NetworkStressTestComponent::OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
    }

    void NetworkStressTestComponent::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
    }

    void NetworkStressTestComponentController::HandleSpawnAIEntity(
        AzNetworking::IConnection* invokingConnection,
        const float& fireIntervalMinMs,
        const float& fireIntervalMaxMs,
        const float& actionIntervalMinMs,
        const float& actionIntervalMaxMs,
        const uint64_t& seed,
        [[maybe_unused]] const int& teamId)
    {
        if (GetSpawnCount() > GetMaxSpawns())
        {
            return;
        }
        ModifySpawnCount()++;

        static Multiplayer::PrefabEntityId prefabId(AZ::Name{ "prefabs/player.network.spawnable" });

        Multiplayer::INetworkEntityManager::EntityList entityList =
            AZ::Interface<Multiplayer::IMultiplayer>::Get()->GetNetworkEntityManager()->CreateEntitiesImmediate(
                prefabId, Multiplayer::NetEntityRole::Authority, AZ::Transform::CreateIdentity(), Multiplayer::AutoActivate::DoNotActivate);

        for (const Multiplayer::NetworkEntityHandle& entityItem : entityList)
        {
            entityItem.GetNetBindComponent()->EnablePlayerHostAutonomy(true);
        }

        Multiplayer::NetworkEntityHandle createdEntity = entityList[0];
        // Drive inputs from AI instead of user inputs and disable camera following
        NetworkAiComponentController* networkAiController = createdEntity.FindController<NetworkAiComponentController>();
        networkAiController->ConfigureAi(fireIntervalMinMs, fireIntervalMaxMs, actionIntervalMinMs, actionIntervalMaxMs, seed);
        networkAiController->SetEnabled(true);
        if (invokingConnection)
        {
            createdEntity.GetNetBindComponent()->SetOwningConnectionId(invokingConnection->GetConnectionId());
        }
        createdEntity.Activate();
    }
} // namespace MultiplayerSample
