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
#if AZ_TRAIT_CLIENT
#include <StartingPointInput/InputEventNotificationBus.h>
#endif

namespace MultiplayerSample
{
    class UiMatchPlayerCoinCountsComponent
        : public AZ::Component
#if AZ_TRAIT_CLIENT
        , public StartingPointInput::InputEventNotificationBus::MultiHandler
#endif
    {
    public:
        AZ_COMPONENT(UiMatchPlayerCoinCountsComponent, "{529b9b3b-bea2-4120-9089-c4451438e4c0}");

        static void Reflect(AZ::ReflectContext* context);

        void Activate() override;
        void Deactivate() override;

#if AZ_TRAIT_CLIENT
        //! StartingPointInput::InputEventNotificationBus overrides ...
        //! @{
        void OnPressed(float value) override;
        void OnReleased(float value) override;
        //! @}
#endif

    private:
        AZ::EntityId m_rootElementId;
        AZStd::vector<AZ::EntityId> m_playerRowElement;

#if AZ_TRAIT_CLIENT
        PlayerNameString GetPlayerName(Multiplayer::NetEntityId playerEntity);
#endif
    };
} // namespace MultiplayerSample
