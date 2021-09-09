/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of
 * this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <Source/AutoGen/StressTestComponent.AutoComponent.h>

#if defined(IMGUI_ENABLED)
#include <imgui/imgui.h>
#include <ImGuiBus.h>
#endif

namespace MultiplayerSample
{
    class StressTestComponent
        : public StressTestComponentBase
    {
    public:
        AZ_MULTIPLAYER_COMPONENT(StressTestComponent, s_stressTestComponentConcreteUuid, StressTestComponentBase);

        static void Reflect(AZ::ReflectContext* context);

        void OnInit() override;
        void OnActivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
        void OnDeactivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
    };

    class StressTestComponentController
        : public StressTestComponentControllerBase
#if defined(IMGUI_ENABLED)
        , public ImGui::ImGuiUpdateListenerBus::Handler
#endif
    {
    public:
        using StressTestComponentControllerBase::StressTestComponentControllerBase;

        void OnActivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
        void OnDeactivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;

        void HandleSpawnAIEntity(
            AzNetworking::IConnection* invokingConnection,
            const float& fireIntervalMinMs,
            const float& fireIntervalMaxMs,
            const float& actionIntervalMinMs,
            const float& actionIntervalMaxMs,
            const uint64_t& seed,
            const int& teamId);

#if defined(IMGUI_ENABLED)
        void OnImGuiMainMenuUpdate() override;
        void OnImGuiUpdate() override;
#endif

    private:
#if defined(IMGUI_ENABLED)
        void DrawEntitySpawner();

        bool m_displayEntitySpawner = false;
        bool m_isServer = false;
        int m_quantity = 1;
        float m_fireIntervalMinMs = 100.f;
        float m_fireIntervalMaxMs = 10000.f;
        float m_actionIntervalMinMs = 500.f;
        float m_actionIntervalMaxMs = 10000.f;
        uint64_t m_seed = 0;
        int m_teamID = 0;
#endif
    };
} // namespace MultiplayerSample
