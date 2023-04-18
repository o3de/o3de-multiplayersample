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
    //! These are all of the user settings that MPS supports.
    struct MpsSettings
    {
        AZ_TYPE_INFO(MpsSettings, "{1E545ABF-6650-41D8-AC69-9C50BB5561F0}");
        static void Reflect(AZ::ReflectContext* context);

        //! The API type that Atom should use at startup. (This value comes from AZ::RHI::APIIndex)
        uint32_t m_atomApiType = 0;

        //! The master audio volume (0 - 100). 0 is silent, 100 is max volume.
        uint8_t m_masterVolume = 100;

        //! The streaming image texture mip bias (0 - N). This affects the max mipmap level that will be loaded for streaming images.
        //! This doesn't affect other types of images like UI or VFX.
        uint8_t m_streamingImageMipBias = 1;
    };

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

        UiToggle m_graphicsApiToggle;
        UiToggle m_textureQualityToggle;
        UiToggle m_masterVolumeToggle;

        MpsSettings m_settings;
    };
}
