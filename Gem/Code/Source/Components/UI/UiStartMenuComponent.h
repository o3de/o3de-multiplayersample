/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/Component/Component.h>
#include <AzCore/Component/TickBus.h>
#include <AzCore/Settings/SettingsRegistry.h>

namespace MultiplayerSample
{
    class UiStartMenuComponent
        : public AZ::Component
        , AZ::TickBus::Handler
    {
    public:
        AZ_COMPONENT(MultiplayerSample::UiStartMenuComponent, "{2F9DA138-1750-4FC9-B1AE-7945D2C1AB4D}");

        /*
        * Reflects component data into the reflection contexts, including the serialization, edit, and behavior contexts.
        */
        static void Reflect(AZ::ReflectContext* context);

    protected:
        void Activate() override;
        void Deactivate() override;

    private:
        //! AZ::TickBus::Handler overrides...
        void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;

        void OnButtonClicked(AZ::EntityId buttonEntityId) const;

        AZ::EntityId m_hostButtonUi;
        AZ::EntityId m_joinButtonUi;
        AZ::EntityId m_exitButtonUi;
        AZ::EntityId m_ipAddressTextInputUi;
        AZ::EntityId m_attemptConnectionBlockerUi;
    };
} // namespace MultiplayerSample
