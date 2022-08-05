/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <UiCoinCountBus.h>
#include <AzCore/Component/Component.h>
#include <AzCore/EBus/ScheduledEvent.h>

namespace MultiplayerSample
{
    class UiCoinCountComponent
        : public AZ::Component
        , public UiCoinCountNotificationBus::Handler
    {
    public:
        AZ_COMPONENT(UiCoinCountComponent, "{ede1871e-70fa-4d63-9dc4-aa451b1b3fa9}");
        
        static void Reflect(AZ::ReflectContext* context);
        
        void Activate() override;
        void Deactivate() override;

        // UiCoinCountNotificationBus overrides ...
        // @{
        void OnCoinCountChanged(uint16_t totalCoinsCollectedByLocalPlayer) override;
        // }@

    private:
        AZ::EntityId m_coinsTextForLocalPlayer;

        // Color change effect
        // @{
        void OnTick(AZ::TimeMs delta);
        AZ::ScheduledEvent m_gameFrameTick{[this]()
        {
            OnTick(m_gameFrameTick.TimeInQueueMs());
        }, AZ::Name("UiCoinCountComponent")};
        AZ::Color m_recentlyChangedCoinTextColor = AZ::Colors::Green;
        AZ::Color m_coinTextColor = AZ::Colors::White;
        AZ::TimeMs m_coinTextColorEffectDuration{ 300 };
        AZ::TimeMs m_coinTextColorEffect = AZ::Time::ZeroTimeMs;
        // }@
    };
} // namespace MultiplayerSample
