/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/Component/Component.h>
#include <Source/Components/NetworkMatchComponent.h>
#include <LyShine/Bus/World/UiCanvasRefBus.h>
#include "MultiplayerSampleTypes.h"

namespace MultiplayerSample
{
    class MatchOverComponent
        : public AZ::Component
        , UiCanvasAssetRefNotificationBus::Handler
    {
    public:
        AZ_COMPONENT(MatchOverComponent, "{f19e68d0-f8a2-4cec-9201-487b5cc697c7}");

        static void Reflect(AZ::ReflectContext* context);

        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);

    protected:
        void Activate() override;
        void Deactivate() override;

    private:
        // UiCanvasAssetRefNotificationBus overrides ...
        void OnCanvasLoadedIntoEntity(AZ::EntityId uiCanvasEntity) override;

        void UpdateRound(uint16_t round);
        void DetermineIfMatchEnded(RoundTimeSec);
        void OnMatchEnd();

        AZ::EntityId m_uiCanvasId;
        uint16_t m_currentRound = 1;
        int m_gameOverElementId = 0;
        int m_hudElementId = 0;
        AZ::EventHandler<uint16_t> m_roundNumberHandler;
        AZ::EventHandler<RoundTimeSec> m_roundTimerHandler;
    };
} // namespace MultiplayerSample