/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once
#include <AzCore/EBus/EBus.h>
#include <AzCore/IO/Path/Path.h>

namespace MultiplayerSample
{
    // This provides a way to get/set every user setting that MultiplayerSample supports, and to save the user settings file.
    // Getting the values pulls them out of the saved user settings data, and setting the values both sets them in the user
    // settings and communicates the change to the appropriate part of the game engine to make the change take effect.
    class MultiplayerSampleUserSettingsRequests
        : public AZ::EBusTraits
    {
    public:
        static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;

        virtual ~MultiplayerSampleUserSettingsRequests() = default;

        // Load the user settings and refresh the game engine based on the settings. They automatically get loaded and applied
        // on launcher startup, but this might need to be called to refresh the settings after connecting to the server and loading
        // the level in case any engine systems get reset by server cvars and level data.
        virtual void Load() = 0;

        // Save the user settings file out to disk.
        virtual void Save() = 0;

        // Change the default graphics API between dx12/vulkan/metal/null on the next restart of the game.
        virtual AZStd::string GetGraphicsApi() = 0;
        virtual void SetGraphicsApi(const AZStd::string& apiName) = 0;

        // Change the master volume from 0 - 100.
        virtual uint8_t GetMasterVolume() = 0;
        virtual void SetMasterVolume(uint8_t masterVolume) = 0;

        // Change the texture quality. 0 = highest quality (highest mipmap), N = lowest quality (lowest mipmap).
        // There's no well-defined value for lowest quality so we'll just arbitrarily cap it at 6 (64x64 if mip 0 is 4096x4096). 
        // Anything lower doesn't really provide any benefit.
        virtual int16_t GetTextureQuality() = 0;
        virtual void SetTextureQuality(int16_t textureQuality) = 0;

        // Change between fullscreen and windowed.
        virtual bool GetFullscreen() = 0;
        virtual void SetFullscreen(bool fullscreen) = 0;

        // Change the rendering resolution (width, height)
        virtual AZStd::pair<uint32_t, uint32_t> GetResolution() = 0;
        virtual void SetResolution(AZStd::pair<uint32_t, uint32_t> resolution) = 0;
    };

    using MultiplayerSampleUserSettingsRequestBus = AZ::EBus<MultiplayerSampleUserSettingsRequests>;

    // This implements the bus provided above. The user settings get auto-loaded at construction and auto-saved at destruction,
    // though saves can also be triggered at other times as well. Because one of the settings is the default graphics API, these
    // settings need to be loaded before system components are initialized because the Atom system components load the graphics
    // API. All of the other settings are changeable at any time and would have allowed this class to get created later in the
    // boot process.
    class MultiplayerSampleUserSettings : public MultiplayerSampleUserSettingsRequestBus::Handler
    {
    public:
        MultiplayerSampleUserSettings();
        ~MultiplayerSampleUserSettings() override;

        void Load() override;
        void Save() override;

        AZStd::string GetGraphicsApi() override;
        void SetGraphicsApi(const AZStd::string& apiName) override;

        uint8_t GetMasterVolume() override;
        void SetMasterVolume(uint8_t masterVolume) override;

        int16_t GetTextureQuality() override;
        void SetTextureQuality(int16_t textureQuality) override;

        bool GetFullscreen() override;
        void SetFullscreen(bool fullscreen) override;


        AZStd::pair<uint32_t, uint32_t> GetResolution() override;
        void SetResolution(AZStd::pair<uint32_t, uint32_t> resolution) override;

    private:
        AZStd::pair<uint32_t, uint32_t> GetMaxResolution();

        using FixedString = AZStd::fixed_string<256>;

        // The base registry key that all our user settings will live underneath.
        // We keep them separate from the rest of the registry hierarchy to ensure that users can't
        // edit their settings file by hand to overwrite any other registry keys that weren't intentionally exposed.
        static inline constexpr FixedString BaseRegistryKey = "/O3DE/MultiplayerSample/User/Settings";

        // These keep track of the specific registry keys used for each setting.
        const FixedString m_graphicsApiKey;
        const FixedString m_textureQualityKey;
        const FixedString m_masterVolumeKey;
        const FixedString m_fullscreenKey;
        const FixedString m_resolutionWidthKey;
        const FixedString m_resolutionHeightKey;

        // The path to the user settings file.
        AZ::IO::FixedMaxPath m_userSettingsPath;

        bool m_changingResolution = false;
    };

} // namespace MultiplayerSample
