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
#include <Atom/RPI.Public/Pass/PassFilter.h>
#include <Atom/RPI.Public/RenderPipeline.h>
#include <Atom/RPI.Public/RPISystemInterface.h>
#include <Atom/RPI.Public/Scene.h>
#include <Atom/Bootstrap/DefaultWindowBus.h>
#include <Atom/Feature/SpecularReflections/SpecularReflectionsFeatureProcessorInterface.h>

#include <AzCore/Console/IConsole.h>
#include <AzCore/IO/GenericStreams.h>
#include <AzCore/Settings/SettingsRegistry.h>
#include <AzCore/Settings/SettingsRegistryMergeUtils.h>
#include <AzCore/Utils/Utils.h>
#include <AzFramework/Entity/GameEntityContextBus.h>
#include <AzFramework/FileFunc/FileFunc.h>
#include <AzFramework/Windowing/WindowBus.h>
#include <IAudioSystem.h>
#include <UserSettings/MultiplayerSampleUserSettings.h>

namespace MultiplayerSample
{
    static constexpr const char* DefaultGraphicsApi = "";       // default to the platform-specific default graphics API
    static constexpr AZ::u64 DefaultVolume[VolumeChannel::Max] =
    {
        100,         // MasterVolume: default to full volume (100)
        100,         // MusicVolume: default to full volume (100)
        100,         // SfxVolume: default to full volume (100)
    };
    static constexpr AZ::s64 DefaultTextureQuality = 1;         // default to one mip level below highest.
    static constexpr bool DefaultFullscreenMode = false;        // default to windowed
    static constexpr AZ::u64 DefaultResolutionWidth = 1920;     // default to 1080p
    static constexpr AZ::u64 DefaultResolutionHeight = 1080;    // default to 1080p
    static constexpr SpecularReflections DefaultReflectionType = SpecularReflections::None;   // default to no reflections
    static constexpr Msaa DefaultMsaa = Msaa::X2;               // default to 2x MSAA
    static constexpr bool DefaultTaa = true;                    // default to TAA enabled


    MultiplayerSampleUserSettings::MultiplayerSampleUserSettings()
        : m_graphicsApiKey(BaseRegistryKey + FixedString("/ApiName"))
        , m_volumeKey{
            BaseRegistryKey + FixedString("/MasterVolume"),
            BaseRegistryKey + FixedString("/MusicVolume"),
            BaseRegistryKey + FixedString("/SfxVolume"),
          }
        , m_textureQualityKey(BaseRegistryKey + FixedString("/TextureQuality"))
        , m_fullscreenKey(BaseRegistryKey + FixedString("/Fullscreen"))
        , m_resolutionWidthKey(BaseRegistryKey + FixedString("/Resolution/Width"))
        , m_resolutionHeightKey(BaseRegistryKey + FixedString("/Resolution/Height"))
        , m_reflectionSettingKey(BaseRegistryKey + FixedString("/Reflections"))
        , m_msaaKey(BaseRegistryKey + FixedString("/MSAA"))
        , m_taaKey(BaseRegistryKey + FixedString("/TAA"))
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

            // Get the current settings values (or the defaults if the keys don't exist) and pass the values back
            // in to set the settings values, which will notify the engine as well as write the keys back into the registry.
            SetGraphicsApi(GetGraphicsApi());
            SetVolume(VolumeChannel::MasterVolume, GetVolume(VolumeChannel::MasterVolume));
            SetVolume(VolumeChannel::MusicVolume, GetVolume(VolumeChannel::MusicVolume));
            SetVolume(VolumeChannel::SfxVolume, GetVolume(VolumeChannel::SfxVolume));
            SetTextureQuality(GetTextureQuality());
            SetFullscreen(GetFullscreen());
            SetResolution(GetResolution());
            SetReflectionSetting(GetReflectionSetting());
            SetMsaa(GetMsaa());
            SetTaa(GetTaa());
        }
    }

    AZStd::string MultiplayerSampleUserSettings::GetGraphicsApi()
    {
        // Default to an empty string, which will just use the default API.
        AZStd::string apiName = DefaultGraphicsApi;

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

    uint8_t MultiplayerSampleUserSettings::GetVolume(VolumeChannel volumeChannel)
    {
        // Default to full volume (100)
        AZ::u64 masterVolume = DefaultVolume[volumeChannel];

        if (auto* registry = AZ::SettingsRegistry::Get(); registry != nullptr)
        {
            registry->Get(masterVolume, m_volumeKey[volumeChannel].c_str());
        }

        // Make sure any hand-edited registry values stay within a valid range.
        return AZStd::clamp(aznumeric_cast<uint8_t>(masterVolume), aznumeric_cast<uint8_t>(0), aznumeric_cast<uint8_t>(100));
    }

    void MultiplayerSampleUserSettings::SetVolume(VolumeChannel volumeChannel, uint8_t masterVolume)
    {
        if (auto* registry = AZ::SettingsRegistry::Get(); registry != nullptr)
        {
            // Send a request to the audio system to change the volume.
            auto audioSystem = AZ::Interface<Audio::IAudioSystem>::Get();
            if (audioSystem)
            {
                static constexpr const char* volumeIds[] =
                {
                    "Volume_Master",
                    "Volume_Music",
                    "Volume_SFX",
                };

                Audio::TAudioObjectID rtpcId = audioSystem->GetAudioRtpcID(volumeIds[volumeChannel]);

                if (rtpcId != INVALID_AUDIO_CONTROL_ID)
                {
                    Audio::ObjectRequest::SetParameterValue setParameter;
                    setParameter.m_audioObjectId = INVALID_AUDIO_OBJECT_ID;
                    setParameter.m_parameterId = rtpcId;
                    // Volume in the audio system is expected to be 0.0 (min) - 1.0 (max), but we're using 0 - 100 as integers,
                    // so convert it from 0 - 100 to the 0 - 1 range.
                    setParameter.m_value = masterVolume / 100.0f;
                    AZ::Interface<Audio::IAudioSystem>::Get()->PushRequest(AZStd::move(setParameter));
                }
            }

            registry->Set(m_volumeKey[volumeChannel].c_str(), aznumeric_cast<AZ::u64>(masterVolume));
        }
    }

    int16_t MultiplayerSampleUserSettings::GetTextureQuality()
    {
        AZ::s64 textureQuality = DefaultTextureQuality;

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

    SpecularReflections MultiplayerSampleUserSettings::GetReflectionSetting()
    {
        AZ::u64 reflectionType = static_cast<AZ::u64>(DefaultReflectionType);

        if (auto* registry = AZ::SettingsRegistry::Get(); registry != nullptr)
        {
            registry->Get(reflectionType, m_reflectionSettingKey.c_str());
        }

        return static_cast<SpecularReflections>(reflectionType);
    }

    void MultiplayerSampleUserSettings::SetReflectionSetting(SpecularReflections reflectionType)
    {
        if (auto* registry = AZ::SettingsRegistry::Get(); registry != nullptr)
        {
            // Only try to set the settings if the scene system is active.
            // If we try to set it too early, it will assert / crash.
            if (AzFramework::SceneSystemInterface::Get())
            {
                AzFramework::EntityContextId entityContextId;
                AzFramework::GameEntityContextRequestBus::BroadcastResult(
                    entityContextId, &AzFramework::GameEntityContextRequestBus::Events::GetGameEntityContextId);

                if (auto scene = AZ::RPI::Scene::GetSceneForEntityContextId(entityContextId); scene)
                {
                    if (auto reflectionFeatureProcessor =
                        scene->GetFeatureProcessor<AZ::Render::SpecularReflectionsFeatureProcessorInterface>(); reflectionFeatureProcessor)
                    {
                        auto ssrOptions = reflectionFeatureProcessor->GetSSROptions();
                        ssrOptions.m_enable = (reflectionType != SpecularReflections::None);
                        ssrOptions.m_rayTracing = (reflectionType == SpecularReflections::ScreenSpaceAndRaytracing);
                        reflectionFeatureProcessor->SetSSROptions(ssrOptions);
                    }
                }
            }

            registry->Set(m_reflectionSettingKey.c_str(), aznumeric_cast<AZ::u64>(reflectionType));
        }
    }

    Msaa MultiplayerSampleUserSettings::GetMsaa()
    {
        AZ::u64 msaa = DefaultMsaa;

        if (auto* registry = AZ::SettingsRegistry::Get(); registry != nullptr)
        {
            registry->Get(msaa, m_msaaKey.c_str());
        }

        return static_cast<Msaa>(msaa);
    }

    void MultiplayerSampleUserSettings::SetMsaa(Msaa msaa)
    {
        if (auto* registry = AZ::SettingsRegistry::Get(); registry != nullptr)
        {
            if (auto rpiSystem = AZ::RPI::RPISystemInterface::Get(); rpiSystem)
            {
                auto multisampleState = rpiSystem->GetApplicationMultisampleState();
                switch (msaa)
                {
                case Msaa::X1:
                    multisampleState.m_samples = 1;
                    break;
                case Msaa::X2:
                    multisampleState.m_samples = 2;
                    break;
                case Msaa::X4:
                    multisampleState.m_samples = 4;
                    break;
                }
                rpiSystem->SetApplicationMultisampleState(multisampleState);
            }

            registry->Set(m_msaaKey.c_str(), aznumeric_cast<AZ::u64>(msaa));
        }
    }

    bool MultiplayerSampleUserSettings::GetTaa()
    {
        bool enabled = DefaultTaa;

        if (auto* registry = AZ::SettingsRegistry::Get(); registry != nullptr)
        {
            registry->Get(enabled, m_taaKey.c_str());
        }

        return enabled;
    }

    void MultiplayerSampleUserSettings::SetTaa(bool enabled)
    {
        if (auto* registry = AZ::SettingsRegistry::Get(); registry != nullptr)
        {
            // Only try to set the settings if the scene system is active.
            // If we try to set it too early, it will assert / crash.
            if (AzFramework::SceneSystemInterface::Get())
            {
                AzFramework::EntityContextId entityContextId;
                AzFramework::GameEntityContextRequestBus::BroadcastResult(
                    entityContextId, &AzFramework::GameEntityContextRequestBus::Events::GetGameEntityContextId);

                if (auto scene = AZ::RPI::Scene::GetSceneForEntityContextId(entityContextId); scene)
                {
                    AZ::RPI::PassFilter passFilter = AZ::RPI::PassFilter::CreateWithPassName(AZ::Name("TaaPass"), scene);
                    AZ::RPI::PassSystemInterface::Get()->ForEachPass(
                        passFilter, [enabled](AZ::RPI::Pass* pass) -> AZ::RPI::PassFilterExecutionFlow
                        {
                            pass->SetEnabled(enabled);
                            return AZ::RPI::PassFilterExecutionFlow::ContinueVisitingPasses;
                        });

                    registry->Set(m_taaKey.c_str(), enabled);
                }
            }
        }
    }

    bool MultiplayerSampleUserSettings::GetFullscreen()
    {
        bool fullscreen = DefaultFullscreenMode;

        if (auto* registry = AZ::SettingsRegistry::Get(); registry != nullptr)
        {
            registry->Get(fullscreen, m_fullscreenKey.c_str());
        }

        return fullscreen;
    }

    void MultiplayerSampleUserSettings::SetFullscreen(bool fullscreen)
    {
        // Because of the way some of our fullscreen/resolution refresh notifications work on the UI settings screen,
        // it's possible to get reentrancy with setting this value. We'll guard against reentrancy so that the top-level
        // setting is the one that sticks.
        if (m_changingResolution)
        {
            return;
        }

        if (auto* registry = AZ::SettingsRegistry::Get(); registry != nullptr)
        {
            if (AZ::IConsole* console = AZ::Interface<AZ::IConsole>::Get(); console)
            {
                AzFramework::NativeWindowHandle windowHandle = nullptr;
                AzFramework::WindowSystemRequestBus::BroadcastResult(
                    windowHandle,
                    &AzFramework::WindowSystemRequestBus::Events::GetDefaultWindowHandle);

                if (!windowHandle)
                {
                    // Initialize the fullscreen state via CVARs if we haven't created the window yet.

                    m_changingResolution = true;

                    AZ::CVarFixedString commandString = AZ::CVarFixedString::format("r_fullscreen %u", fullscreen ? 1 : 0);
                    console->PerformCommand(commandString.c_str());

                    m_changingResolution = false;
                }
                else
                {
                    // Change the existing fullscreen state if the window already exists.

                    bool isFullscreen = false;
                    AzFramework::WindowRequestBus::EventResult(
                        isFullscreen, windowHandle,
                        &AzFramework::WindowRequestBus::Events::GetFullScreenState);

                    if (isFullscreen != fullscreen) 
                    {
                        m_changingResolution = true;

                        AzFramework::WindowRequestBus::Event(
                            windowHandle,
                            &AzFramework::WindowRequestBus::Events::SetFullScreenState, fullscreen);

                        m_changingResolution = false;
                    }
                }

                if (fullscreen)
                {
                    // Once Atom supports setting a rendering resolution to something other than the current window size,
                    // this is where we'd want to make the appropriate API calls to change it when entering fullscreen mode.
                    // Right now, fullscreen mode uses the full monitor resolution as Atom's resolution.
                    // Ideally, we would like Atom to use the resolution that's set in the Resolution user setting regardless
                    // of windowed or fullscreen, and would instead scale to fill the rullscreen real estate.
                }
                else
                {
                    // When leaving fullscreen, set the window resolution to the current requested resolution.
                    // This is necessary because by default, leaving fullscreen will return the window back to its
                    // pre-fullscreen state. But if we've changed the requested resolution between now and then, we
                    // want to make sure we end up with a window that matches the currently-requested resolution instead.
                    SetResolution(GetResolution());
                }
            }

            registry->Set(m_fullscreenKey.c_str(), fullscreen);
        }
    }

    AZStd::pair<uint32_t, uint32_t> MultiplayerSampleUserSettings::GetResolution()
    {
        AZ::u64 width = DefaultResolutionWidth;
        AZ::u64 height = DefaultResolutionHeight;

        if (auto* registry = AZ::SettingsRegistry::Get(); registry != nullptr)
        {
            registry->Get(width, m_resolutionWidthKey.c_str());
            registry->Get(height, m_resolutionHeightKey.c_str());
        }

        return { aznumeric_cast<uint32_t>(width), aznumeric_cast<uint32_t>(height) };
    }

    void MultiplayerSampleUserSettings::SetResolution(AZStd::pair<uint32_t, uint32_t> resolution)
    {
        // Because of the way some of our fullscreen/resolution refresh notifications work on the UI settings screen,
        // it's possible to get reentrancy with setting this value. We'll guard against reentrancy so that the top-level
        // setting is the one that sticks.
        if (m_changingResolution)
        {
            return;
        }

        if (auto* registry = AZ::SettingsRegistry::Get(); registry != nullptr)
        {
            if (AZ::IConsole* console = AZ::Interface<AZ::IConsole>::Get(); console)
            {
                AzFramework::NativeWindowHandle windowHandle = nullptr;
                AzFramework::WindowSystemRequestBus::BroadcastResult(
                    windowHandle,
                    &AzFramework::WindowSystemRequestBus::Events::GetDefaultWindowHandle);

                if (!windowHandle)
                {
                    // Initialize the resolution via CVARs if the window doesn't exist yet.

                    m_changingResolution = true;

                    AZ::CVarFixedString commandString = AZ::CVarFixedString::format("r_width %u", resolution.first);
                    console->PerformCommand(commandString.c_str());

                    commandString = AZ::CVarFixedString::format("r_height %u", resolution.second);
                    console->PerformCommand(commandString.c_str());

                    m_changingResolution = false;
                }
                else
                {
                    bool fullscreen = false;
                    AzFramework::WindowRequestBus::EventResult(
                        fullscreen, windowHandle,
                        &AzFramework::WindowRequestBus::Events::GetFullScreenState);

                    if (!fullscreen)
                    {
                        // If the window exists, and isn't in fullscreen mode, resize it to the requested resolution.
                        // To prevent people from getting into a bad state, also clamp the resolution to the maximum
                        // resolution that fits on the current monitor.

                        auto maxResolution = GetMaxResolution();

                        AzFramework::WindowSize desiredSize = {
                            AZStd::min(resolution.first, maxResolution.first), AZStd::min(resolution.second, maxResolution.second) };

                        AzFramework::WindowSize windowSize = desiredSize;
                        AzFramework::WindowRequestBus::EventResult(
                            windowSize, windowHandle,
                            &AzFramework::WindowRequestBus::Events::GetClientAreaSize);

                        if ((desiredSize.m_height != windowSize.m_height) || (desiredSize.m_width != windowSize.m_width))
                        {
                            m_changingResolution = true;

                            AzFramework::WindowRequestBus::Event(
                                windowHandle,
                                &AzFramework::WindowRequestBus::Events::ResizeClientArea,
                                AzFramework::WindowSize(
                                    AZStd::min(resolution.first, maxResolution.first), AZStd::min(resolution.second, maxResolution.second)),
                                AzFramework::WindowPosOptions());

                            m_changingResolution = false;
                        }
                    }
                    else
                    {
                        m_changingResolution = true;

                        // Once Atom supports setting a rendering resolution to something other than the current window size,
                        // this is where we'd want to make the appropriate API calls to change it when changing resolutions while
                        // in fullscreen mode.
                        // Right now, fullscreen mode uses the full monitor resolution as Atom's resolution.
                        // Ideally, we would like Atom to use the resolution that's set in the Resolution user setting regardless
                        // of windowed or fullscreen, and would instead scale to fill the rullscreen real estate.

                        m_changingResolution = false;
                    }
                }
            }

            registry->Set(m_resolutionWidthKey.c_str(), aznumeric_cast<AZ::u64>(resolution.first));
            registry->Set(m_resolutionHeightKey.c_str(), aznumeric_cast<AZ::u64>(resolution.second));
        }
    }

    AZStd::pair<uint32_t, uint32_t> MultiplayerSampleUserSettings::GetMaxResolution()
    {
        AzFramework::NativeWindowHandle windowHandle = nullptr;
        AzFramework::WindowSystemRequestBus::BroadcastResult(
            windowHandle,
            &AzFramework::WindowSystemRequestBus::Events::GetDefaultWindowHandle);

        AzFramework::WindowSize windowSize = { AZStd::numeric_limits<uint32_t>::max(), AZStd::numeric_limits<uint32_t>::max() };
        AzFramework::WindowRequestBus::EventResult(
            windowSize, windowHandle, &AzFramework::WindowRequestBus::Events::GetMaximumClientAreaSize);

        return { windowSize.m_width, windowSize.m_height };
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
