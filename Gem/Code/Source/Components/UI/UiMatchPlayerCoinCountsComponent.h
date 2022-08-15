/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/Component/Component.h>
#include <AzCore/std/containers/vector.h>
#include <StartingPointInput/InputEventNotificationBus.h>

namespace MultiplayerSample
{
    class UiMatchPlayerCoinCountsComponent
        : public AZ::Component
        , public StartingPointInput::InputEventNotificationBus::MultiHandler
    {
    public:
        AZ_COMPONENT(UiMatchPlayerCoinCountsComponent, "{529b9b3b-bea2-4120-9089-c4451438e4c0}");

        static void Reflect(AZ::ReflectContext* context);

        void Activate() override;
        void Deactivate() override;

        //! StartingPointInput::InputEventNotificationBus overrides ...
        //! @{
        void OnPressed(float value) override;
        void OnReleased(float value) override;
        //! @}

    private:
        AZ::EntityId m_rootElementId;
        AZStd::vector<AZ::EntityId> m_playerRowElement;

        PlayerNameString GetPlayerName(Multiplayer::NetEntityId playerEntity);
    };
} // namespace MultiplayerSample
