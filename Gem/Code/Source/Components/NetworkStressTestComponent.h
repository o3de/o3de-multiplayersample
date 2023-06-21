/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of
 * this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <Source/AutoGen/NetworkStressTestComponent.AutoComponent.h>

#if defined(IMGUI_ENABLED)
#include <imgui/imgui.h>
#include <ImGuiBus.h>
#endif

namespace MultiplayerSample
{
    class NetworkStressTestComponent
        : public NetworkStressTestComponentBase
    {
    public:
        AZ_MULTIPLAYER_COMPONENT(MultiplayerSample::NetworkStressTestComponent, s_networkStressTestComponentConcreteUuid, MultiplayerSample::NetworkStressTestComponentBase);

        static void Reflect(AZ::ReflectContext* context);

        void OnInit() override;
        void OnActivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
        void OnDeactivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
    };

    class NetworkStressTestComponentController
        : public NetworkStressTestComponentControllerBase
#if defined(IMGUI_ENABLED)
        , public ImGui::ImGuiUpdateListenerBus::Handler
#endif
    {
    public:
        using NetworkStressTestComponentControllerBase::NetworkStressTestComponentControllerBase;

        NetworkStressTestComponentController(NetworkStressTestComponent& owner);

        void OnActivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
        void OnDeactivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;

#if AZ_TRAIT_SERVER
        void HandleSpawnAiEntity();

        void HandleSpawnAIEntity(
            AzNetworking::IConnection* invokingConnection,
            const float& fireIntervalMinMs,
            const float& fireIntervalMaxMs,
            const float& actionIntervalMinMs,
            const float& actionIntervalMaxMs,
            const int& teamId);
#endif

#if defined(IMGUI_ENABLED)
        void OnImGuiMainMenuUpdate() override;
        void OnImGuiUpdate() override;
#endif

    private:
#if defined(IMGUI_ENABLED)
        void DrawEntitySpawner();

        bool m_displayEntitySpawner = false;
#endif
        [[maybe_unused]] bool m_isServer = false;
        [[maybe_unused]] int m_quantity = 1;
        [[maybe_unused]] float m_fireIntervalMinMs = 100.f;
        [[maybe_unused]] float m_fireIntervalMaxMs = 10000.f;
        [[maybe_unused]] float m_actionIntervalMinMs = 500.f;
        [[maybe_unused]] float m_actionIntervalMaxMs = 10000.f;
        [[maybe_unused]] int m_teamID = 0;

#if AZ_TRAIT_SERVER
        AZ::ScheduledEvent m_autoSpawnTimer;
#endif
    };
} // namespace MultiplayerSample
