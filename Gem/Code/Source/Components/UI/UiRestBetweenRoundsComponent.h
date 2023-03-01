/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once


#include <AzCore/Component/Component.h>

#include <UiRoundsLifecycleBus.h>

namespace MultiplayerSample
{
    class UiRestBetweenRoundsComponent
        : public AZ::Component
        , public UiRoundsLifecycleBus::Handler
    {
    public:
        AZ_COMPONENT(UiRestBetweenRoundsComponent, "{8BF185B2-DCE7-462B-B151-43E0AF717BA5}");

        static void Reflect(AZ::ReflectContext* context);

        void Activate() override;
        void Deactivate() override;

#if AZ_TRAIT_CLIENT
        //! UiRoundsLifecycleBus overrides
        //! @{
        void OnRoundRestTimeRemainingChanged(RoundTimeSec secondsRenaming) override;
        //! }@
#endif

    private:
        AZ::EntityId m_restTimerRootUiElement;
        AZ::EntityId m_numbersContainerUiElement;
    };
}