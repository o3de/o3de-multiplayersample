/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <Atom/RPI.Public/Image/ImageSystem.h>
#include <Atom/RPI.Public/Image/StreamingImage.h>
#include <Atom/RPI.Public/Image/StreamingImagePool.h>

#include <AzCore/Console/IConsole.h>
#include <AzCore/IO/GenericStreams.h>
#include <AzCore/Settings/SettingsRegistry.h>
#include <AzCore/Settings/SettingsRegistryMergeUtils.h>
#include <AzCore/Utils/Utils.h>
#include <AzFramework/FileFunc/FileFunc.h>
#include <AzFramework/Windowing/WindowBus.h>
#include <IAudioSystem.h>
#include <UserSettings/MultiplayerSampleUserSettings.h>

namespace MultiplayerSample
{
    MultiplayerSampleUserSettings::MultiplayerSampleUserSettings()
        : m_graphicsApiKey(BaseRegistryKey + FixedString("/ApiName"))
        , m_masterVolumeKey(BaseRegistryKey + FixedString("/MasterVolume"))
        , m_textureQualityKey(BaseRegistryKey + FixedString("/TextureQuality"))
        , m_fullscreenKey(BaseRegistryKey + FixedString("/Fullscreen"))
        , m_resolutionWidthKey(BaseRegistryKey + FixedString("/Resolution/Width"))
        , m_resolutionHeightKey(BaseRegistryKey + FixedString("/Resolution/Height"))
    {
        MultiplayerSampleUserSettingsRequestBus::Handler::BusConnect();

        // Create a full path including filename for the user settings file.
        m_userSettingsPath = AZ::Utils::GetProjectUserPath();
        m_userSettingsPath /= "Registry";
        m_userSettingsPath /= "MultiplayerSampleUserSettings.setreg";

        // Load all of our settings keys, create default values if they don't exist and initialize the engine settings as appropriate.
        Load();
    }

    MultiplayerSampleUserSettings::~MultiplayerSampleUserSettings()
    {
        MultiplayerSampleUserSettingsRequestBus::Handler::BusDisconnect();

        // Always auto-save the user settings on destruction.
        Save();
    }

    void MultiplayerSampleUserSettings::Load()
    {
        if (auto* registry = AZ::SettingsRegistry::Get(); registry != nullptr)
        {
            // Read the setreg file from a loose file into a string in memory. This isn't technically a "cfg" file,
            // but the method does the exact set of steps needed here to read a loose file into memory, so even though
            // it has a slightly misleading name, it keeps us from duplicating the code.
            AZ::Outcome<AZStd::string, AZStd::string> userSettings = 
                AzFramework::FileFunc::GetCfgFileContents(AZStd::string(m_userSettingsPath.FixedMaxPathString()));

            if (userSettings.IsSuccess())
            {
                // Merge the user settings file under the base "/O3DE/MultiplayerSample/User/Settings" key.
                // This will ensure that it cannot overwrite any other engine settings.
                // MergeSettings() is used here instead of MergeSettingsFile() because MergeSettingsFile() uses
                // FileIOBase to read in the file, which will attempt to read it from a PAK file in PAK file builds.
                // Our settings file will always be a loose file, so we instead read it into a buffer beforehand and then
                // apply it here from the in-memory buffer.
                [[maybe_unused]] auto mergeSuccess = registry->MergeSettings(userSettings.GetValue(),
                    AZ::SettingsRegistryInterface::Format::JsonMergePatch, BaseRegistryKey);

                AZ_Error("UserSettings", mergeSuccess, "Failed to merge user settings into the O3DE registry.");
            }

            // Get the current settings values or the defaults if the keys don't exist.
            AZStd::string apiName = GetGraphicsApi();
            uint8_t masterVolume = GetMasterVolume();
            int16_t textureQuality = GetTextureQuality();
            bool fullscreen = GetFullscreen();
            AZStd::pair<uint32_t, uint32_t> resolution = GetResolution();

            // Set the settings values, which will notify the engine as well as write the keys back into the registry.
            SetGraphicsApi(apiName);
            SetMasterVolume(masterVolume);
            SetTextureQuality(textureQuality);
            SetFullscreen(fullscreen);
            SetResolution(resolution);
        }
    }

    AZStd::string MultiplayerSampleUserSettings::GetGraphicsApi()
    {
        // Default to an empty string, which will just use the default API.
        AZStd::string apiName;

        if (auto* registry = AZ::SettingsRegistry::Get(); registry != nullptr)
        {
            registry->Get(apiName, m_graphicsApiKey.c_str());
        }

        return apiName;
    }

    void MultiplayerSampleUserSettings::SetGraphicsApi(const AZStd::string& apiName)
    {
        if (auto* registry = AZ::SettingsRegistry::Get(); registry != nullptr)
        {
            // Set the requested api name as the highest (and only) user priority in the registry.
            // Atom will select this api at startup as long as it exists and nothing was passed in via command-line.
            // If the passed-in apiName is empty, just let Atom use its standard default priorities for api selection.
            // If the passed-in apiName doesn't match one supported by Atom on this platform, Atom will ignore it and use
            // its standard default priorities as well.
            if (!apiName.empty())
            {
                AZStd::vector<AZStd::string> factoriesPriority;
                factoriesPriority.emplace_back(apiName);
                registry->SetObject("/O3DE/Atom/RHI/FactoryManager/factoriesPriority", factoriesPriority);
            }

            registry->Set(m_graphicsApiKey.c_str(), apiName);
        }
    }

    uint8_t MultiplayerSampleUserSettings::GetMasterVolume()
    {
        // Default to full volume (100)
        AZ::u64 masterVolume = 100;

        if (auto* registry = AZ::SettingsRegistry::Get(); registry != nullptr)
        {
            registry->Get(masterVolume, m_masterVolumeKey.c_str());
        }

        // Make sure any hand-edited registry values stay within a valid range.
        return AZStd::clamp(aznumeric_cast<uint8_t>(masterVolume), aznumeric_cast<uint8_t>(0), aznumeric_cast<uint8_t>(100));
    }

    void MultiplayerSampleUserSettings::SetMasterVolume(uint8_t masterVolume)
    {
        if (auto* registry = AZ::SettingsRegistry::Get(); registry != nullptr)
        {
            // Send a request to the audio system to change the master volume.
            auto audioSystem = AZ::Interface<Audio::IAudioSystem>::Get();
            if (audioSystem)
            {
                Audio::TAudioObjectID rtpcId = audioSystem->GetAudioRtpcID("Volume_Master");

                if (rtpcId != INVALID_AUDIO_CONTROL_ID)
                {
                    Audio::ObjectRequest::SetParameterValue setParameter;
                    setParameter.m_audioObjectId = INVALID_AUDIO_OBJECT_ID;
                    setParameter.m_parameterId = rtpcId;
                    // Master volume in the audio system is expected to be 0.0 (min) - 1.0 (max), but we're using 0 - 100 as integers,
                    // so convert it from 0 - 100 to the 0 - 1 range.
                    setParameter.m_value = masterVolume / 100.0f;
                    AZ::Interface<Audio::IAudioSystem>::Get()->PushRequest(AZStd::move(setParameter));
                }
            }

            registry->Set(m_masterVolumeKey.c_str(), aznumeric_cast<AZ::u64>(masterVolume));
        }
    }

    int16_t MultiplayerSampleUserSettings::GetTextureQuality()
    {
        AZ::s64 textureQuality = 1;

        if (auto* registry = AZ::SettingsRegistry::Get(); registry != nullptr)
        {
            registry->Get(textureQuality, m_textureQualityKey.c_str());
        }

        return AZStd::clamp(aznumeric_cast<int16_t>(textureQuality), aznumeric_cast<int16_t>(0), aznumeric_cast<int16_t>(10));
    }

    void MultiplayerSampleUserSettings::SetTextureQuality(int16_t textureQuality)
    {
        if (auto* registry = AZ::SettingsRegistry::Get(); registry != nullptr)
        {
            if (auto* imageSystem = AZ::RPI::ImageSystemInterface::Get())
            {
                AZ::Data::Instance<AZ::RPI::StreamingImagePool> pool = imageSystem->GetSystemStreamingPool();
                pool->SetMipBias(textureQuality);
            }

            registry->Set(m_textureQualityKey.c_str(), aznumeric_cast<AZ::s64>(textureQuality));
        }
    }

    bool MultiplayerSampleUserSettings::GetFullscreen()
    {
        bool fullscreen = false;

        if (auto* registry = AZ::SettingsRegistry::Get(); registry != nullptr)
        {
            registry->Get(fullscreen, m_fullscreenKey.c_str());
        }

        return fullscreen;
    }

    void MultiplayerSampleUserSettings::SetFullscreen(bool fullscreen)
    {
        if (auto* registry = AZ::SettingsRegistry::Get(); registry != nullptr)
        {
            if (AZ::IConsole* console = AZ::Interface<AZ::IConsole>::Get(); console)
            {
                // Change the fullscreen state if we haven't created the window yet.
                AZ::CVarFixedString commandString = AZ::CVarFixedString::format("r_fullscreen %u", fullscreen ? 1 : 0);
                console->PerformCommand(commandString.c_str());

                // Change the fullscreen state if the window already exists
                AzFramework::NativeWindowHandle windowHandle = nullptr;
                AzFramework::WindowSystemRequestBus::BroadcastResult(
                    windowHandle,
                    &AzFramework::WindowSystemRequestBus::Events::GetDefaultWindowHandle);

                AzFramework::WindowRequestBus::Event(
                    windowHandle,
                    &AzFramework::WindowRequestBus::Events::SetFullScreenState, fullscreen);
            }

            registry->Set(m_fullscreenKey.c_str(), fullscreen);
        }
    }

    AZStd::pair<uint32_t, uint32_t> MultiplayerSampleUserSettings::GetResolution()
    {
        AZ::u64 width = 1920;
        AZ::u64 height = 1080;

        if (auto* registry = AZ::SettingsRegistry::Get(); registry != nullptr)
        {
            registry->Get(width, m_resolutionWidthKey.c_str());
            registry->Get(height, m_resolutionHeightKey.c_str());
        }

        return { aznumeric_cast<uint32_t>(width), aznumeric_cast<uint32_t>(height) };
    }

    void MultiplayerSampleUserSettings::SetResolution(AZStd::pair<uint32_t, uint32_t> resolution)
    {
        if (auto* registry = AZ::SettingsRegistry::Get(); registry != nullptr)
        {
            if (AZ::IConsole* console = AZ::Interface<AZ::IConsole>::Get(); console)
            {
                // This will technically change the window resolution to whatever is requrested, but it should 
                // ideally take into account the current DPI scaling and what the maximum resolution of the monitor is.
                
                // Change the resolution if the window doesn't exist yet.
                AZ::CVarFixedString commandString = AZ::CVarFixedString::format("r_width %u", resolution.first);
                console->PerformCommand(commandString.c_str());

                commandString = AZ::CVarFixedString::format("r_height %u", resolution.second);
                console->PerformCommand(commandString.c_str());

                // Change the resolution if the window already exists.
                AzFramework::NativeWindowHandle windowHandle = nullptr;
                AzFramework::WindowSystemRequestBus::BroadcastResult(
                    windowHandle,
                    &AzFramework::WindowSystemRequestBus::Events::GetDefaultWindowHandle);

                bool fullscreen = false;
                AzFramework::WindowRequestBus::EventResult(
                    fullscreen, windowHandle,
                    &AzFramework::WindowRequestBus::Events::GetFullScreenState);

                // Don't resize if we're in fullscreen mode.
                if (!fullscreen)
                {
                    AzFramework::WindowRequestBus::Event(
                        windowHandle,
                        &AzFramework::WindowRequestBus::Events::ResizeClientArea,
                        AzFramework::WindowSize(resolution.first, resolution.second), AzFramework::WindowPosOptions());
                }
            }

            registry->Set(m_resolutionWidthKey.c_str(), aznumeric_cast<AZ::u64>(resolution.first));
            registry->Set(m_resolutionHeightKey.c_str(), aznumeric_cast<AZ::u64>(resolution.second));
        }
    }

    void MultiplayerSampleUserSettings::Save()
    {
        AZ::IO::FixedMaxPath userSettingsSavePath = m_userSettingsPath;
        userSettingsSavePath.ReplaceExtension("setreg.tmp");

        constexpr AZ::IO::OpenMode openMode = AZ::IO::OpenMode::ModeWrite | AZ::IO::OpenMode::ModeCreatePath;

        // Write to a temporary file and then move the file to the final location
        if (AZ::IO::SystemFileStream userSettingsStream(userSettingsSavePath.c_str(), openMode); userSettingsStream.IsOpen())
        {
            auto settingsRegistry = AZ::SettingsRegistry::Get();

            // Remove the .tmp extension from the user settings path
            // This results in the final path where the settings will actually be saved
            userSettingsSavePath.ReplaceExtension();
            AZ::SettingsRegistryMergeUtils::DumperSettings dumperSettings;
            dumperSettings.m_prettifyOutput = true;
            if (AZ::SettingsRegistryMergeUtils::DumpSettingsRegistryToStream(
                *settingsRegistry, BaseRegistryKey, userSettingsStream, dumperSettings))
            {
                // Use SystemFile::Rename to move the file to the final destination
                userSettingsStream.Close();
                bool renameSuccess = AZ::IO::SystemFile::Rename(userSettingsStream.GetFilename(), userSettingsSavePath.c_str(), true);
                AZ_Error("UserSettings", renameSuccess, 
                    "Renaming '%s' to '%s' failed.", userSettingsStream.GetFilename(), userSettingsSavePath.c_str());
            }
        }
    }

} // namespace MultiplayerSample
