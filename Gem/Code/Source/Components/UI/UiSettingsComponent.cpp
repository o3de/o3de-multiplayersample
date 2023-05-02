/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <Atom/RHI/Factory.h>
#include <AzCore/Interface/Interface.h>
#include <AzCore/Serialization/EditContext.h>
#include <LyShine/Bus/UiButtonBus.h>
#include <LyShine/Bus/UiElementBus.h>
#include <LyShine/Bus/UiTextBus.h>
#include <Source/Components/UI/UiSettingsComponent.h>
#include <Source/UserSettings/MultiplayerSampleUserSettings.h>

namespace MultiplayerSample
{
    void UiToggle::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<UiToggle>()
                ->Version(0)
                ->Field("Label", &UiToggle::m_labelEntity)
                ->Field("LeftButton", &UiToggle::m_leftButtonEntity)
                ->Field("RightButton", &UiToggle::m_rightButtonEntity)
                ;

            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                editContext->Class<UiToggle>("Ui Toggle", "Manages the user settings")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &UiToggle::m_labelEntity, "Label", "The toggle's label entity.")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &UiToggle::m_leftButtonEntity, "Left Button", "The toggle's left button entity.")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &UiToggle::m_rightButtonEntity, "Right Button", "The toggle's right button entity.")
                    ;
            }
        }
    }

    void UiSettingsComponent::Reflect(AZ::ReflectContext* context)
    {
        UiToggle::Reflect(context);

        if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<UiSettingsComponent, AZ::Component>()
                ->Version(0)
                ->Field("GraphicsApi", &UiSettingsComponent::m_graphicsApiToggle)
                ->Field("TextureQuality", &UiSettingsComponent::m_textureQualityToggle)
                ->Field("MasterVolume", &UiSettingsComponent::m_masterVolumeToggle)
                ->Field("Fullscreen", &UiSettingsComponent::m_fullscreenToggle)
                ->Field("Resolution", &UiSettingsComponent::m_resolutionToggle)
                ;

            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                editContext->Class<UiSettingsComponent>("Ui Settings", "Manages the user settings")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC_CE("CanvasUI"))
                    ->DataElement(AZ::Edit::UIHandlers::Default, &UiSettingsComponent::m_graphicsApiToggle, "Graphics Api", "The Graphics Api toggle elements.")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &UiSettingsComponent::m_textureQualityToggle, "Texture Quality", "The Texture Quality toggle elements.")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &UiSettingsComponent::m_masterVolumeToggle, "Master Volume", "The Master Volume toggle elements.")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &UiSettingsComponent::m_fullscreenToggle, "Fullscreen", "The Fullscreen toggle elements.")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &UiSettingsComponent::m_resolutionToggle, "Resolution", "The Resolution toggle elements.")
                    ;
            }
        }
    }

    AzFramework::NativeWindowHandle UiSettingsComponent::GetWindowHandle()
    {
        AzFramework::NativeWindowHandle windowHandle = nullptr;
        AzFramework::WindowSystemRequestBus::BroadcastResult(
            windowHandle,
            &AzFramework::WindowSystemRequestBus::Events::GetDefaultWindowHandle);

        return windowHandle;
    }

    void UiSettingsComponent::Activate()
    {
        // Listen for window notifications so that we can detect fullscreen/windowed changes.
        AzFramework::WindowNotificationBus::Handler::BusConnect(GetWindowHandle());

        // Loads and applies the current user settings when this component activates.
        // The user settings should *already* be loaded and applied at Launcher startup, but connecting to the server
        // and switching levels can cause some engine settings to reset themselves, so this will reapply the desired
        // user settings again.
        MultiplayerSampleUserSettingsRequestBus::Broadcast(&MultiplayerSampleUserSettingsRequestBus::Events::Load);

        // Initialize the toggles to the current values
        InitializeToggle(m_graphicsApiToggle, OnGraphicsApiToggle);
        InitializeToggle(m_textureQualityToggle, OnTextureQualityToggle);
        InitializeToggle(m_masterVolumeToggle, OnMasterVolumeToggle);
        InitializeToggle(m_fullscreenToggle, OnFullscreenToggle);
        InitializeToggle(m_resolutionToggle, OnResolutionToggle);
    }

    void UiSettingsComponent::Deactivate()
    {
        AzFramework::WindowNotificationBus::Handler::BusDisconnect();
    }

    void UiSettingsComponent::InitializeToggle(UiToggle& toggle, AZStd::function<void(UiToggle&, ToggleDirection)> toggleUpdateFn)
    {
        toggleUpdateFn(toggle, ToggleDirection::None);

        UiButtonBus::Event(toggle.m_leftButtonEntity, &UiButtonInterface::SetOnClickCallback,
            [&toggle, toggleUpdateFn]([[maybe_unused]] AZ::EntityId buttonEntityId, [[maybe_unused]] AZ::Vector2 position)
            {
                toggleUpdateFn(toggle, ToggleDirection::Left);
            });
        UiButtonBus::Event(toggle.m_rightButtonEntity, &UiButtonInterface::SetOnClickCallback,
            [&toggle, toggleUpdateFn]([[maybe_unused]] AZ::EntityId buttonEntityId, [[maybe_unused]] AZ::Vector2 position)
            {
                toggleUpdateFn(toggle, ToggleDirection::Right);
            });
    }

    template<typename ValueType>
    uint32_t UiSettingsComponent::GetRotatedIndex(
        const AZStd::vector<AZStd::pair<ValueType, AZStd::string>>& valuesToLabels,
        const ValueType& value, ToggleDirection toggleDirection)
    {
        const size_t totalValues = valuesToLabels.size();

        uint32_t curIndex = 0;

        // Loop through and look for the correct value
        for (size_t index = 0; index < totalValues; index++)
        {
            if (value == valuesToLabels[index].first)
            {
                curIndex = aznumeric_cast<uint32_t>(index);
                break;
            }
        }

        switch (toggleDirection)
        {
        case ToggleDirection::Left:
            return aznumeric_cast<uint32_t>((curIndex + (totalValues - 1)) % totalValues);
        case ToggleDirection::Right:
            return aznumeric_cast<uint32_t>((curIndex + 1) % totalValues);
        default:
            return curIndex;
        }
    }

    void UiSettingsComponent::OnGraphicsApiToggle(UiToggle& toggle, ToggleDirection toggleDirection)
    {
        // This list is expected to match the values in AZ::RHI::ApiIndex.
        const AZStd::vector<AZStd::pair<AZStd::string, AZStd::string>> valuesToLabels =
        {
            { "null", "Null" },
            { "dx12", "DirectX 12" },
            { "vulkan", "Vulkan" },
            { "metal", "Metal" }
        };

        // Get the current api selection.
        AZStd::string graphicsApi;
        MultiplayerSampleUserSettingsRequestBus::BroadcastResult(
            graphicsApi, &MultiplayerSampleUserSettingsRequestBus::Events::GetGraphicsApi);

        // If there isn't anything stored in the user settings yet, default to the currently-loaded api.
        if (graphicsApi.empty())
        {
            graphicsApi = AZ::RHI::Factory::Get().GetName().GetStringView();
        }

        // Rotate the index based on toggle direction.
        uint32_t graphicsApiIndex = GetRotatedIndex(valuesToLabels, graphicsApi, toggleDirection);

        UiTextBus::Event(toggle.m_labelEntity, &UiTextInterface::SetText, valuesToLabels[graphicsApiIndex].second);

        MultiplayerSampleUserSettingsRequestBus::Broadcast(
            &MultiplayerSampleUserSettingsRequestBus::Events::SetGraphicsApi, valuesToLabels[graphicsApiIndex].first);

        MultiplayerSampleUserSettingsRequestBus::Broadcast(&MultiplayerSampleUserSettingsRequestBus::Events::Save);
    }

    void UiSettingsComponent::OnTextureQualityToggle(UiToggle& toggle, ToggleDirection toggleDirection)
    {
        const AZStd::vector<AZStd::pair<int16_t, AZStd::string>> valuesToLabels =
        {
            { aznumeric_cast<int16_t>(6), "Rock Bottom (64)" },
            { aznumeric_cast<int16_t>(5), "Extremely Low (128)" },
            { aznumeric_cast<int16_t>(4), "Very Low (256)" },
            { aznumeric_cast<int16_t>(3), "Low (512)" },
            { aznumeric_cast<int16_t>(2), "Medium (1K)" },
            { aznumeric_cast<int16_t>(1), "High (2K)" },
            { aznumeric_cast<int16_t>(0), "Ultra (4K)" },
        };

        // Get the current texture quality value.
        int16_t textureQuality = 0;
        MultiplayerSampleUserSettingsRequestBus::BroadcastResult(
            textureQuality, &MultiplayerSampleUserSettingsRequestBus::Events::GetTextureQuality);

        // Rotate the index based on toggle direction.
        uint32_t textureQualityIndex = GetRotatedIndex(valuesToLabels, textureQuality, toggleDirection);

        UiTextBus::Event(toggle.m_labelEntity, &UiTextInterface::SetText, valuesToLabels[textureQualityIndex].second);

        MultiplayerSampleUserSettingsRequestBus::Broadcast(
            &MultiplayerSampleUserSettingsRequestBus::Events::SetTextureQuality, valuesToLabels[textureQualityIndex].first);

        MultiplayerSampleUserSettingsRequestBus::Broadcast(&MultiplayerSampleUserSettingsRequestBus::Events::Save);
    }

    void UiSettingsComponent::OnMasterVolumeToggle(UiToggle& toggle, ToggleDirection toggleDirection)
    {
        const AZStd::vector<AZStd::pair<uint8_t, AZStd::string>> valuesToLabels =
        {
            { aznumeric_cast<uint8_t>(0), "0 (off)" },
            { aznumeric_cast<uint8_t>(10), "10" },
            { aznumeric_cast<uint8_t>(20), "20" },
            { aznumeric_cast<uint8_t>(30), "30" },
            { aznumeric_cast<uint8_t>(40), "40" },
            { aznumeric_cast<uint8_t>(50), "50" },
            { aznumeric_cast<uint8_t>(60), "60" },
            { aznumeric_cast<uint8_t>(70), "70" },
            { aznumeric_cast<uint8_t>(80), "80" },
            { aznumeric_cast<uint8_t>(90), "90" },
            { aznumeric_cast<uint8_t>(100), "100 (max)" },
        };

        // Get the current master volume value.
        uint8_t masterVolume = 0;
        MultiplayerSampleUserSettingsRequestBus::BroadcastResult(
            masterVolume, &MultiplayerSampleUserSettingsRequestBus::Events::GetMasterVolume);

        // Make sure our master volume is a multiple of 10.
        masterVolume = (masterVolume / 10) * 10;

        // Rotate the index based on toggle direction.
        uint32_t masterVolumeIndex = GetRotatedIndex(valuesToLabels, masterVolume, toggleDirection);

        UiTextBus::Event(toggle.m_labelEntity, &UiTextInterface::SetText, valuesToLabels[masterVolumeIndex].second);

        MultiplayerSampleUserSettingsRequestBus::Broadcast(
            &MultiplayerSampleUserSettingsRequestBus::Events::SetMasterVolume, valuesToLabels[masterVolumeIndex].first);

        MultiplayerSampleUserSettingsRequestBus::Broadcast(&MultiplayerSampleUserSettingsRequestBus::Events::Save);
    }

    void UiSettingsComponent::OnFullscreenToggle(UiToggle& toggle, ToggleDirection toggleDirection)
    {
        const AZStd::vector<AZStd::pair<bool, AZStd::string>> valuesToLabels =
        {
            { false, "Windowed" },
            { true, "Fullscreen" },
        };

        // Get the current fullscreen state. Unlike the other settings, we'll get this from the current window state so that we
        // handle things like Alt-enter that can change our windowing state regardless of what our user settings thinks.

        // Start by getting the user setting as our default state.
        bool fullscreen = false;
        MultiplayerSampleUserSettingsRequestBus::BroadcastResult(
            fullscreen, &MultiplayerSampleUserSettingsRequestBus::Events::GetFullscreen);

        // Next, try to get the current state from the window. If it fails to get the state, we'll default to the
        // user setting value that we fetched above.
        bool currentFullscreenState = fullscreen;
        AzFramework::WindowRequestBus::EventResult(currentFullscreenState,
            GetWindowHandle(), &AzFramework::WindowRequestBus::Events::GetFullScreenState);

        // Rotate the index based on toggle direction.
        uint32_t fullscreenIndex = GetRotatedIndex(valuesToLabels, currentFullscreenState, toggleDirection);

        UiTextBus::Event(toggle.m_labelEntity, &UiTextInterface::SetText, valuesToLabels[fullscreenIndex].second);

        // Only set & save the state if the value has changed. We might get a lot of spurious notifications here, so
        // we'll be more conservative with this setting than with the other settings.
        if (valuesToLabels[fullscreenIndex].first != currentFullscreenState)
        {
            MultiplayerSampleUserSettingsRequestBus::Broadcast(
                &MultiplayerSampleUserSettingsRequestBus::Events::SetFullscreen, valuesToLabels[fullscreenIndex].first);

            MultiplayerSampleUserSettingsRequestBus::Broadcast(&MultiplayerSampleUserSettingsRequestBus::Events::Save);
        }
    }

    void UiSettingsComponent::OnResolutionToggle(UiToggle& toggle, ToggleDirection toggleDirection)
    {
        const AZStd::vector<AZStd::pair<AZStd::pair<uint32_t, uint32_t>, AZStd::string>> valuesToLabels =
        {
            // This set of resolutions was selected because they're the set that we offer in O3DE in the IMGUI debug menus.
            // Feel free to add, remove or reorder resolution pairs from this list as appropriate.
            // There is logic below for filtering out resolutions that are too large that assumes that the smallest 
            // resolution is the first entry on the list, so just keep that in mind when altering this list.

            { { 800, 334}, "800 x 334 (43:18)" },
            { { 800, 450}, "800 x 450 (16:9)" },
            { { 800, 500}, "800 x 500 (16:10)" },
            { { 800, 600}, "800 x 600 (4:3)" },

            { {1280, 535}, "1280 x 535 (43:18)" },
            { {1280, 720}, "1280 x 720 (16:9)" },
            { {1280, 800}, "1280 x 800 (16:10)" },
            { {1280, 960}, "1280 x 960 (4:3)" },

            { {1600, 669}, "1600 x 669 (43:18)" },
            { {1600, 900}, "1600 x 900 (16:9)" },
            { {1600, 1000}, "1600 x 1000 (16:10)" },
            { {1600, 1200}, "1600 x 1200 (4:3)" },

            { {1920, 803}, "1920 x 803 (43:18)" },
            { {1920, 1080}, "1920 x 1080 (16:9)" },
            { {1920, 1200}, "1920 x 1200 (16:10)" },
            { {1920, 1440}, "1920 x 1440 (4:3)" },

            { {2560, 1071}, "2560 x 1071 (43:18)" },
            { {2560, 1440}, "2560 x 1440 (16:9)" },
            { {2560, 1600}, "2560 x 1600 (16:10)" },
            { {2560, 1920}, "2560 x 1920 (4:3)" },

            { {3440, 1440}, "3440 x 1440 (43:18)" },
            { {3440, 1935}, "3440 x 1935 (16:9)" },
            { {3440, 2150}, "3440 x 2150 (16:10)" },
            { {3440, 2580}, "3440 x 2580 (4:3)" },

            { {3840, 1607}, "3840 x 1607 (43:18)" },
            { {3840, 2160}, "3840 x 2160 (16:9)" },
            { {3840, 2400}, "3840 x 2400 (16:10)" },
            { {3840, 2880}, "3840 x 2880 (4:3)" },
        };

        // Get the max supported resolution for the monitor that the window is currently on.
        AzFramework::WindowSize maxWindowSize = { AZStd::numeric_limits<uint32_t>::max(), AZStd::numeric_limits<uint32_t>::max() };
        AzFramework::WindowRequestBus::EventResult(maxWindowSize,
            GetWindowHandle(), &AzFramework::WindowRequestBus::Events::GetMaximumClientAreaSize);

        // Get the current resolution value.
        AZStd::pair<uint32_t, uint32_t> resolution = { 1920, 1080 };
        MultiplayerSampleUserSettingsRequestBus::BroadcastResult(
            resolution, &MultiplayerSampleUserSettingsRequestBus::Events::GetResolution);

        // Rotate the index based on toggle direction.
        uint32_t resolutionIndex = GetRotatedIndex(valuesToLabels, resolution, toggleDirection);

        // If the resolution is too big for the monitor, keep rotating the index until we find one that fits
        // or until we reach the start of the list. The list starts with the smallest resolution, so if that one 
        // doesn't fit, there's no point in continuing to look for something smaller.
        while ((resolutionIndex > 0) && (
            (valuesToLabels[resolutionIndex].first.first > maxWindowSize.m_width) ||
            (valuesToLabels[resolutionIndex].first.second > maxWindowSize.m_height)))
        {
            ToggleDirection searchDirection = (toggleDirection == ToggleDirection::None) ? ToggleDirection::Left : toggleDirection;
            resolutionIndex = GetRotatedIndex(valuesToLabels, valuesToLabels[resolutionIndex].first, searchDirection);
        }

        UiTextBus::Event(toggle.m_labelEntity, &UiTextInterface::SetText, valuesToLabels[resolutionIndex].second);

        MultiplayerSampleUserSettingsRequestBus::Broadcast(
            &MultiplayerSampleUserSettingsRequestBus::Events::SetResolution, valuesToLabels[resolutionIndex].first);

        MultiplayerSampleUserSettingsRequestBus::Broadcast(&MultiplayerSampleUserSettingsRequestBus::Events::Save);
    }

    void UiSettingsComponent::OnFullScreenModeChanged([[maybe_unused]] bool fullscreen)
    {
        // Refresh the windowed / fullscreen setting and toggle in case the user changes the current mode by pressing Alt-Enter.
        OnFullscreenToggle(m_fullscreenToggle, ToggleDirection::None);
    }
}
