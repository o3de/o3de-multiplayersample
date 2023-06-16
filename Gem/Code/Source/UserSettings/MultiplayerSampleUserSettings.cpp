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

    using FixedString = AZStd::fixed_string<256>;

    // The base registry key that all our user settings will live underneath.
    // We keep them separate from the rest of the registry hierarchy to ensure that users can't
    // edit their settings file by hand to overwrite any other registry keys that weren't intentionally exposed.
    static constexpr FixedString BaseRegistryKey = "/O3DE/MultiplayerSample/User/Settings";

    // These keep track of the specific registry keys used for each setting.
    static constexpr FixedString GraphicsApiKey(BaseRegistryKey + FixedString("/ApiName"));
    static constexpr FixedString TextureQualityKey(BaseRegistryKey + FixedString("/TextureQuality"));
    static constexpr FixedString VolumeKey[VolumeChannel::Max]
    {
            BaseRegistryKey + FixedString("/MasterVolume"),
            BaseRegistryKey + FixedString("/MusicVolume"),
            BaseRegistryKey + FixedString("/SfxVolume"),
    };
    static constexpr FixedString FullscreenKey(BaseRegistryKey + FixedString("/Fullscreen"));
    static constexpr FixedString ResolutionWidthKey(BaseRegistryKey + FixedString("/Resolution/Width"));
    static constexpr FixedString ResolutionHeightKey(BaseRegistryKey + FixedString("/Resolution/Height"));
    static constexpr FixedString ReflectionSettingKey(BaseRegistryKey + FixedString("/Reflections"));
    static constexpr FixedString MsaaKey(BaseRegistryKey + FixedString("/MSAA"));
    static constexpr FixedString TaaKey(BaseRegistryKey + FixedString("/TAA"));

    MultiplayerSampleUserSettings::MultiplayerSampleUserSettings()
    {
        m_registry = AZ::SettingsRegistry::Get();
        AZ_Assert(m_registry, "Initialization order incorrect, MultiplayerSampleUserSettings has somehow started before "
            "the Settings Registry. Initial settings won't get applied correctly.");

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
        AZ::Render::Bootstrap::NotificationBus::Handler::BusDisconnect();
        MultiplayerSampleUserSettingsRequestBus::Handler::BusDisconnect();

        // Always auto-save the user settings on destruction.
        Save();
    }

    void MultiplayerSampleUserSettings::ApplyMsaaSetting()
    {
        // To apply the MSAA setting at the correct point in the boot process, we need to wait until the bootstrap scene is ready.
        // To listen for that, we need to connect to the Bootstrap::NotificationBus. However, we can't connect to that bus until
        // the scene system and main scene are created, which happens in the activation of the AzFrameworkConfigurationSystemComponent.

        // So, we'll have the MultiplayerSampleSystemComponent depend on the AzFrameworkConfigurationSystemComponent, then on 
        // activation, call ApplyMsaaSetting(), which tells us to connect to the Bootstrap::NotificationBus, which then will tell
        // us when the bootstrap scene is ready so that we can apply the MSAA setting.

        // If we're ever able to change the MSAA setting at runtime, we can remove this entire flow and just call 
        // SetMsaaInRenderer() from the SetMsaa() call.

        AZ::Render::Bootstrap::NotificationBus::Handler::BusConnect();
    }

    void MultiplayerSampleUserSettings::OnBootstrapSceneReady([[maybe_unused]] AZ::RPI::Scene* bootstrapScene)
    {
        // Only set the MSAA setting at boot time. Changing it at runtime can lead to crashes.
        SetMsaaInRenderer(GetMsaa());
    }

    void MultiplayerSampleUserSettings::Load()
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
            [[maybe_unused]] auto mergeSuccess = m_registry->MergeSettings(userSettings.GetValue(),
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

    AZStd::string MultiplayerSampleUserSettings::GetGraphicsApi()
    {
        // Default to an empty string, which will just use the default API.
        AZStd::string apiName = DefaultGraphicsApi;
        m_registry->Get(apiName, GraphicsApiKey.c_str());

        return apiName;
    }

    void MultiplayerSampleUserSettings::SetGraphicsApi(const AZStd::string& apiName)
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
            m_registry->SetObject("/O3DE/Atom/RHI/FactoryManager/factoriesPriority", factoriesPriority);
        }

        m_registry->Set(GraphicsApiKey.c_str(), apiName);
    }

    uint8_t MultiplayerSampleUserSettings::GetVolume(VolumeChannel volumeChannel)
    {
        // Default to full volume (100)
        AZ::u64 masterVolume = DefaultVolume[volumeChannel];
        m_registry->Get(masterVolume, VolumeKey[volumeChannel].c_str());

        // Make sure any hand-edited registry values stay within a valid range.
        return AZStd::clamp(aznumeric_cast<uint8_t>(masterVolume), aznumeric_cast<uint8_t>(0), aznumeric_cast<uint8_t>(100));
    }

    void MultiplayerSampleUserSettings::SetVolume(VolumeChannel volumeChannel, uint8_t masterVolume)
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

        m_registry->Set(VolumeKey[volumeChannel].c_str(), aznumeric_cast<AZ::u64>(masterVolume));
    }

    int16_t MultiplayerSampleUserSettings::GetTextureQuality()
    {
        AZ::s64 textureQuality = DefaultTextureQuality;
        m_registry->Get(textureQuality, TextureQualityKey.c_str());

        return AZStd::clamp(aznumeric_cast<int16_t>(textureQuality), aznumeric_cast<int16_t>(0), aznumeric_cast<int16_t>(10));
    }

    void MultiplayerSampleUserSettings::SetTextureQuality(int16_t textureQuality)
    {
        if (auto* imageSystem = AZ::RPI::ImageSystemInterface::Get())
        {
            AZ::Data::Instance<AZ::RPI::StreamingImagePool> pool = imageSystem->GetSystemStreamingPool();
            pool->SetMipBias(textureQuality);
        }

        m_registry->Set(TextureQualityKey.c_str(), aznumeric_cast<AZ::s64>(textureQuality));
    }

    SpecularReflections MultiplayerSampleUserSettings::GetReflectionSetting()
    {
        AZ::u64 reflectionType = static_cast<AZ::u64>(DefaultReflectionType);
        m_registry->Get(reflectionType, ReflectionSettingKey.c_str());

        return static_cast<SpecularReflections>(reflectionType);
    }

    void MultiplayerSampleUserSettings::SetReflectionSetting(SpecularReflections reflectionType)
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

        m_registry->Set(ReflectionSettingKey.c_str(), aznumeric_cast<AZ::u64>(reflectionType));
    }

    Msaa MultiplayerSampleUserSettings::GetMsaa()
    {
        AZ::u64 msaa = static_cast<AZ::u64>(DefaultMsaa);
        m_registry->Get(msaa, MsaaKey.c_str());

        return static_cast<Msaa>(msaa);
    }

    void MultiplayerSampleUserSettings::SetMsaaInRenderer(Msaa msaa)
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
    }

    void MultiplayerSampleUserSettings::SetMsaa(Msaa msaa)
    {
        // Currently MSAA settings don't get changed at runtime because they have the potential
        // to cause a TDR graphics driver crash on at least Vulkan, maybe others.
        // This might be the result of mixing PSOs between the msaa variant and the non-msaa variant in the same frame.
        // Until the problem gets tracked down and resolved, we'll only set the MSAA setting in the renderer at boot time.
        // If this ever gets fixed, the call to SetMsaaInRenderer() can get moved to here, and the extra flow handling to call
        // it sooner can get removed.

        m_registry->Set(MsaaKey.c_str(), aznumeric_cast<AZ::u64>(msaa));
    }

    bool MultiplayerSampleUserSettings::GetTaa()
    {
        bool enabled = DefaultTaa;
        m_registry->Get(enabled, TaaKey.c_str());

        return enabled;
    }

    void MultiplayerSampleUserSettings::SetTaa(bool enabled)
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

                m_registry->Set(TaaKey.c_str(), enabled);
            }
        }
    }

    bool MultiplayerSampleUserSettings::GetFullscreen()
    {
        bool fullscreen = DefaultFullscreenMode;
        m_registry->Get(fullscreen, FullscreenKey.c_str());

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

        if (AZ::IConsole* console = AZ::Interface<AZ::IConsole>::Get(); console)
        {
            AzFramework::NativeWindowHandle windowHandle = nullptr;
            AzFramework::WindowSystemRequestBus::BroadcastResult(
                windowHandle,
                &AzFramework::WindowSystemRequestBus::Events::GetDefaultWindowHandle);

            auto resolution = GetResolution();

            if (!windowHandle)
            {
                // Initialize the fullscreen state via CVARs if we haven't created the window yet.

                m_changingResolution = true;

                AZ::CVarFixedString commandString = AZ::CVarFixedString::format("r_fullscreen %u", fullscreen ? 1 : 0);
                console->PerformCommand(commandString.c_str());

                // set resolution mode to 1 if it's full screen (use specified resolution)
                // set it to 0 if it's not full screen (use window area size)
                commandString = AZ::CVarFixedString::format("r_resolutionMode %u", fullscreen ? 1 : 0);
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

                AzFramework::WindowRequestBus::Event(
                    windowHandle,
                    &AzFramework::WindowRequestBus::Events::SetEnableCustomizedResolution, fullscreen);

                if (isFullscreen != fullscreen) 
                {
                    m_changingResolution = true;

                    AzFramework::WindowRequestBus::Event(
                        windowHandle,
                        &AzFramework::WindowRequestBus::Events::SetFullScreenState, fullscreen);
                    m_changingResolution = false;
                }

                if (fullscreen)
                {
                    AzFramework::WindowRequestBus::Event(
                        windowHandle,
                        &AzFramework::WindowRequestBus::Events::SetRenderResolution, AzFramework::WindowSize(resolution.first, resolution.second));
                }
            }
                
            if (!fullscreen)
            {
                // When leaving fullscreen, set the window resolution to the current requested resolution.
                // This is necessary because by default, leaving fullscreen will return the window back to its
                // pre-fullscreen state. But if we've changed the requested resolution between now and then, we
                // want to make sure we end up with a window that matches the currently-requested resolution instead.
                SetResolution(resolution);
            }
        }

        m_registry->Set(FullscreenKey.c_str(), fullscreen);
    }

    AZStd::pair<uint32_t, uint32_t> MultiplayerSampleUserSettings::GetResolution()
    {
        AZ::u64 width = DefaultResolutionWidth;
        AZ::u64 height = DefaultResolutionHeight;

        m_registry->Get(width, ResolutionWidthKey.c_str());
        m_registry->Get(height, ResolutionHeightKey.c_str());

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

                     AzFramework::WindowRequestBus::Event(
                            windowHandle,
                            &AzFramework::WindowRequestBus::Events::SetRenderResolution,
                            AzFramework::WindowSize(resolution.first, resolution.second)
                            );

                    m_changingResolution = false;
                }
            }
        }

        m_registry->Set(ResolutionWidthKey.c_str(), aznumeric_cast<AZ::u64>(resolution.first));
        m_registry->Set(ResolutionHeightKey.c_str(), aznumeric_cast<AZ::u64>(resolution.second));
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
                [[maybe_unused]] bool renameSuccess = AZ::IO::SystemFile::Rename(userSettingsStream.GetFilename(), userSettingsSavePath.c_str(), true);
                AZ_Error("UserSettings", renameSuccess, 
                    "Renaming '%s' to '%s' failed.", userSettingsStream.GetFilename(), userSettingsSavePath.c_str());
            }
        }
    }

} // namespace MultiplayerSample
