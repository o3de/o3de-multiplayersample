/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once


#include <AzCore/Component/Component.h>
#include <AzFramework/Windowing/WindowBus.h>
#include <Source/UserSettings/MultiplayerSampleUserSettings.h>


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

    // This component is attached to the UI Settings screen and handles the logic for changing the user setting values
    // when the UI toggles are toggled. It activates on level load, not on settings screen navigation as one might think.
    // On activation, it reapplies all of the MPS user settings while initializing the toggles to help ensure that the
    // previously-applied settings weren't overridden by the server or by level loads.
    class UiSettingsComponent
        : public AZ::Component
        , public AzFramework::WindowNotificationBus::Handler
    {
    public:
        AZ_COMPONENT(UiSettingsComponent, "{6F0F5495-E766-444C-808E-4EB91AD891D6}");

        static void Reflect(AZ::ReflectContext* context);

        void Activate() override;
        void Deactivate() override;
    private:
        // WindowNotificationBus overrides
        void OnFullScreenModeChanged(bool fullscreen) override;

        static AzFramework::NativeWindowHandle GetWindowHandle();

        enum class ToggleDirection
        {
            None,
            Left,
            Right
        };

        void InitializeToggle(UiToggle& toggle, AZStd::function<void(UiToggle&, ToggleDirection)> toggleUpdateFn);

        static void OnGraphicsApiToggle(UiToggle& toggle, ToggleDirection toggleDirection);
        static void OnTextureQualityToggle(UiToggle& toggle, ToggleDirection toggleDirection);
        static void OnMasterVolumeToggle(UiToggle& toggle, ToggleDirection toggleDirection);
        static void OnMusicVolumeToggle(UiToggle& toggle, ToggleDirection toggleDirection);
        static void OnSfxVolumeToggle(UiToggle& toggle, ToggleDirection toggleDirection);
        static void OnVolumeToggle(VolumeChannel volumeChannel, UiToggle& toggle, ToggleDirection toggleDirection);
        static void OnFullscreenToggle(UiToggle& toggle, ToggleDirection toggleDirection);
        static void OnResolutionToggle(UiToggle& toggle, ToggleDirection toggleDirection);
        static void OnReflectionToggle(UiToggle& toggle, ToggleDirection toggleDirection);
        static void OnMsaaToggle(UiToggle& toggle, ToggleDirection toggleDirection);
        static void OnTaaToggle(UiToggle& toggle, ToggleDirection toggleDirection);

        template<typename ValueType>
        static uint32_t GetRotatedIndex(
            const AZStd::span<const AZStd::pair<ValueType, AZStd::string_view>> valuesToLabels,
            const ValueType& value, ToggleDirection toggleDirection);

        UiToggle m_graphicsApiToggle;
        UiToggle m_textureQualityToggle;
        UiToggle m_fullscreenToggle;
        UiToggle m_resolutionToggle;

        UiToggle m_reflectionToggle;
        UiToggle m_msaaToggle;
        UiToggle m_taaToggle;

        UiToggle m_masterVolumeToggle;
        UiToggle m_musicVolumeToggle;
        UiToggle m_sfxVolumeToggle;
    };
}
