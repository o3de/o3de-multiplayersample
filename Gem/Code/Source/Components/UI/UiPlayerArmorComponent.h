/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <UiPlayerArmorBus.h>
#include <PlayerIdentityBus.h>
#include <AzCore/Component/Component.h>

namespace MultiplayerSample
{
    class UiPlayerArmorComponent
        : public AZ::Component
        , public UiPlayerArmorNotificationBus::Handler
        , public PlayerIdentityNotificationBus::Handler
    {
    public:
        AZ_COMPONENT(UiPlayerArmorComponent, "{15de84e4-eb35-4c9a-a0e3-9e39c10a7ff4}");

        static void Reflect(AZ::ReflectContext* context);

        void Activate() override;
        void Deactivate() override;

        //! UiPlayerArmorNotificationBus overrides ...
        //! @{
        void OnPlayerArmorChanged(float armorPointsForLocalPlayer, float startingArmor) override;
        //! @}

        //! PlayerIdentityNotificationBus overrides ...
        //! @{
        void OnAutonomousPlayerNameChanged(const char* playerName) override;
        //! @}

    private:
        // UI entities
        AZ::EntityId m_rootElement;
        AZ::EntityId m_playerName;
        AZ::EntityId m_armorVisualEntity;
        AZ::EntityId m_armorText;
    };
} // namespace MultiplayerSample
