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
#include <Atom/Bootstrap/BootstrapNotificationBus.h>

namespace AZ
{
    class SettingsRegistryInterface;
}

namespace MultiplayerSample
{
    enum VolumeChannel : uint8_t
    {
        MasterVolume,
        MusicVolume,
        SfxVolume,
        Max
    };

    enum class SpecularReflections : uint8_t
    {
        None,
        ScreenSpace,
        ScreenSpaceAndRaytracing
    };

    enum class Msaa : uint8_t
    {
        X1,
        X2,
        X4
    };

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

        // Change the type of screen space reflections
        virtual SpecularReflections GetReflectionSetting() = 0;
        virtual void SetReflectionSetting(SpecularReflections reflectionType) = 0;

        // Change the MSAA setting
        virtual Msaa GetMsaa() = 0;
        virtual void SetMsaa(Msaa msaa) = 0;

        // This is a workaround. The MSAA setting can currently only be applied at boot time or else
        // it has the potential to lead to a graphics crash.
        virtual void ApplyMsaaSetting() = 0;

        // Enable/Disable TAA
        virtual bool GetTaa() = 0;
        virtual void SetTaa(bool enable) = 0;

        // Change the volume setting from 0 - 100 for the given channel
        virtual uint8_t GetVolume(VolumeChannel volumeChannel) = 0;
        virtual void SetVolume(VolumeChannel volumeChannel, uint8_t volume) = 0;
    };

    using MultiplayerSampleUserSettingsRequestBus = AZ::EBus<MultiplayerSampleUserSettingsRequests>;

    // This implements the bus provided above. The user settings get auto-loaded at construction and auto-saved at destruction,
    // though saves can also be triggered at other times as well. Because one of the settings is the default graphics API, these
    // settings need to be loaded before system components are initialized because the Atom system components load the graphics
    // API. All of the other settings are changeable at any time and would have allowed this class to get created later in the
    // boot process.
    class MultiplayerSampleUserSettings 
        : public MultiplayerSampleUserSettingsRequestBus::Handler
        , public AZ::Render::Bootstrap::NotificationBus::Handler
    {
    public:
        MultiplayerSampleUserSettings();
        ~MultiplayerSampleUserSettings() override;

        // AZ::Render::Bootstrap::NotificationBus overrides...
        void OnBootstrapSceneReady(AZ::RPI::Scene* bootstrapScene) override;

        // MultiplayerSampleUserSettingsRequestBus overrides...
        void Load() override;
        void Save() override;

        AZStd::string GetGraphicsApi() override;
        void SetGraphicsApi(const AZStd::string& apiName) override;

        int16_t GetTextureQuality() override;
        void SetTextureQuality(int16_t textureQuality) override;

        bool GetFullscreen() override;
        void SetFullscreen(bool fullscreen) override;

        SpecularReflections GetReflectionSetting() override;
        void SetReflectionSetting(SpecularReflections reflectionType) override;

        Msaa GetMsaa() override;
        void SetMsaa(Msaa msaa) override;
        void ApplyMsaaSetting() override;

        bool GetTaa() override;
        void SetTaa(bool enable) override;

        uint8_t GetVolume(VolumeChannel volumeChannel) override;
        void SetVolume(VolumeChannel volumeChannel, uint8_t volume) override;

        AZStd::pair<uint32_t, uint32_t> GetResolution() override;
        void SetResolution(AZStd::pair<uint32_t, uint32_t> resolution) override;

    private:
        void SetMsaaInRenderer(Msaa msaa);
        AZStd::pair<uint32_t, uint32_t> GetMaxResolution();

        // Cached pointer to the settings registry so that we don't have to fetch it for every setting.
        AZ::SettingsRegistryInterface* m_registry = nullptr;

        // The path to the user settings file.
        AZ::IO::FixedMaxPath m_userSettingsPath;

        bool m_changingResolution = false;
    };

} // namespace MultiplayerSample
