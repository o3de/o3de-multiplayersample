/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of
 * this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <Source/Components/StressTestComponent.h>

#include <Source/Components/NetworkAiComponent.h>
#include <Source/Components/WasdPlayerMovementComponent.h>

#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <Multiplayer/IMultiplayer.h>
#include <Multiplayer/Components/NetBindComponent.h>
#include <Multiplayer/ConnectionData/IConnectionData.h>
#include <Multiplayer/ReplicationWindows/IReplicationWindow.h>

namespace MultiplayerSample
{
    void StressTestComponent::Reflect(AZ::ReflectContext* context)
    {
        AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context);
        if (serializeContext)
        {
            serializeContext->Class<StressTestComponent, StressTestComponentBase>()
                ->Version(1);
        }

        StressTestComponentBase::Reflect(context);
    }

    void StressTestComponent::OnInit()
    {
    }

    void StressTestComponentController::OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
#ifdef IMGUI_ENABLED
        ImGui::ImGuiUpdateListenerBus::Handler::BusConnect();
#endif
        auto agentType = AZ::Interface<Multiplayer::IMultiplayer>::Get()->GetAgentType();
        switch (agentType)
        {
        case Multiplayer::MultiplayerAgentType::DedicatedServer:
        case Multiplayer::MultiplayerAgentType::ClientServer:
            m_isServer = true;
            break;
        default:
            break;
        }
    }

    void StressTestComponentController::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
#ifdef IMGUI_ENABLED
        ImGui::ImGuiUpdateListenerBus::Handler::BusDisconnect();
#endif
    }

#if defined(IMGUI_ENABLED)
    void StressTestComponentController::OnImGuiMainMenuUpdate()
    {
        if (ImGui::BeginMenu("Multiplayer Sample"))
        {
            ImGui::Checkbox("Entity Spawner", &m_displayEntitySpawner);
            ImGui::EndMenu();
        }
    }

    void StressTestComponentController::OnImGuiUpdate()
    {
        if (m_displayEntitySpawner)
        {
            if (ImGui::Begin("Entity Spawner", &m_displayEntitySpawner, ImGuiWindowFlags_None))
            {
                DrawEntitySpawner();
            }
        }
    }

    void StressTestComponentController::DrawEntitySpawner()
    {
        ImGui::SliderInt("Quantity", &m_quantity, 1, 100);
        ImGui::SliderInt("Team ID", &m_teamID, 0, 3);
        if (ImGui::Button("Spawn AI Entity"))
        {
            for (int i = 0; i != m_quantity; ++i)
            {
                if (m_isServer)
                {
                    HandleSpawnAIEntity(nullptr, m_teamID);
                }
                else
                {
                    SpawnAIEntity(m_teamID);
                }
            }
        }
    }
#endif

    void StressTestComponent::OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
    }

    void StressTestComponent::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
    }

    void StressTestComponentController::HandleSpawnAIEntity(
        AzNetworking::IConnection* invokingConnection, [[maybe_unused]] const int& teamId)
    {
        static Multiplayer::PrefabEntityId prefabId(AZ::Name{ "prefabs/player.network.spawnable" });

        Multiplayer::INetworkEntityManager::EntityList entityList =
            AZ::Interface<Multiplayer::IMultiplayer>::Get()->GetNetworkEntityManager()->CreateEntitiesImmediate(
            prefabId, Multiplayer::NetEntityRole::Authority, AZ::Transform::CreateIdentity(), Multiplayer::AutoActivate::DoNotActivate);

        Multiplayer::NetworkEntityHandle createdEntity = entityList[0];
        // Drive inputs from AI instead of user inputs and disable camera following
        NetworkAiComponentController* networkAiController =
            reinterpret_cast<NetworkAiComponentController*>(createdEntity.FindComponent<NetworkAiComponent>()->GetController());
        networkAiController->SetEnabled(true);
        if (invokingConnection)
        {
            networkAiController->SetOwningConnectionId(static_cast<uint32_t>(invokingConnection->GetConnectionId()));
            createdEntity.GetNetBindComponent()->SetOwningConnectionId(invokingConnection->GetConnectionId());
        }
        else
        {
            // Server-owned AI component
            networkAiController->SetOwningConnectionId(0xffffff);
        }
        createdEntity.GetNetBindComponent()->SetAllowAutonomy(true);
        createdEntity.Activate();
    }
} // namespace MultiplayerSample
