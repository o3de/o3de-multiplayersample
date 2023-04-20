/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once


#include <AzCore/Component/Component.h>

namespace MultiplayerSample
{
    struct UiToggle
    {
        AZ_TYPE_INFO(UiToggle, "{60AD7DDE-1730-41D8-BB82-630FF8008370}");
        static void Reflect(AZ::ReflectContext* context);

        AZ::EntityId m_labelEntity;
        AZ::EntityId m_leftButtonEntity;
        AZ::EntityId m_rightButtonEntity;
    };

    class UiSettingsComponent
        : public AZ::Component
    {
    public:
        AZ_COMPONENT(UiSettingsComponent, "{6F0F5495-E766-444C-808E-4EB91AD891D6}");

        static void Reflect(AZ::ReflectContext* context);

        void Activate() override;
        void Deactivate() override;
    private:
        enum class ToggleDirection
        {
            None,
            Left,
            Right
        };

        void OnGraphicsApiToggle(ToggleDirection toggleDirection);
        void OnTextureQualityToggle(ToggleDirection toggleDirection);
        void OnMasterVolumeToggle(ToggleDirection toggleDirection);

        template<typename ValueType>
        static uint32_t GetRotatedIndex(
            const AZStd::vector<AZStd::pair<ValueType, AZStd::string>>& valuesToLabels,
            const ValueType& value, ToggleDirection toggleDirection);

        UiToggle m_graphicsApiToggle;
        UiToggle m_textureQualityToggle;
        UiToggle m_masterVolumeToggle;
    };
}
