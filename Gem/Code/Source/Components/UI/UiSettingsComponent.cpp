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
                ;

            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                editContext->Class<UiSettingsComponent>("Ui Settings", "Manages the user settings")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC_CE("CanvasUI"))
                    ->DataElement(AZ::Edit::UIHandlers::Default, &UiSettingsComponent::m_graphicsApiToggle, "Graphics Api", "The Graphics Api toggle elements.")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &UiSettingsComponent::m_textureQualityToggle, "Texture Quality", "The Texture Quality toggle elements.")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &UiSettingsComponent::m_masterVolumeToggle, "Master Volume", "The Master Volume toggle elements.")
                    ;
            }
        }
    }

    void UiSettingsComponent::Activate()
    {
        // Make sure to load and apply the current user settings when this component activates.
        // The user settings should *already* be loaded and applied at Launcher startup, but connecting to the server
        // and switching levels can cause some engine settings to reset themselves, so this will reapply the desired
        // user settings again.
        MultiplayerSampleUserSettingsRequestBus::Broadcast(&MultiplayerSampleUserSettingsRequestBus::Events::Load);

        // Initialize the toggles to the current values
        OnGraphicsApiToggle(ToggleDirection::None);
        OnTextureQualityToggle(ToggleDirection::None);
        OnMasterVolumeToggle(ToggleDirection::None);

        // Start listening for button presses
        UiButtonBus::Event(m_graphicsApiToggle.m_leftButtonEntity, &UiButtonInterface::SetOnClickCallback,
            [this]([[maybe_unused]] AZ::EntityId buttonEntityId, [[maybe_unused]] AZ::Vector2 position) 
            { 
                OnGraphicsApiToggle(ToggleDirection::Left); 
            });
        UiButtonBus::Event(m_graphicsApiToggle.m_rightButtonEntity, &UiButtonInterface::SetOnClickCallback,
            [this]([[maybe_unused]] AZ::EntityId buttonEntityId, [[maybe_unused]] AZ::Vector2 position)
            {
                OnGraphicsApiToggle(ToggleDirection::Right);
            });
        UiButtonBus::Event(m_textureQualityToggle.m_leftButtonEntity, &UiButtonInterface::SetOnClickCallback,
            [this]([[maybe_unused]] AZ::EntityId buttonEntityId, [[maybe_unused]] AZ::Vector2 position)
            {
                OnTextureQualityToggle(ToggleDirection::Left);
            });
        UiButtonBus::Event(m_textureQualityToggle.m_rightButtonEntity, &UiButtonInterface::SetOnClickCallback,
            [this]([[maybe_unused]] AZ::EntityId buttonEntityId, [[maybe_unused]] AZ::Vector2 position)
            {
                OnTextureQualityToggle(ToggleDirection::Right);
            });
        UiButtonBus::Event(m_masterVolumeToggle.m_leftButtonEntity, &UiButtonInterface::SetOnClickCallback,
            [this]([[maybe_unused]] AZ::EntityId buttonEntityId, [[maybe_unused]] AZ::Vector2 position)
            {
                OnMasterVolumeToggle(ToggleDirection::Left);
            });
        UiButtonBus::Event(m_masterVolumeToggle.m_rightButtonEntity, &UiButtonInterface::SetOnClickCallback,
            [this]([[maybe_unused]] AZ::EntityId buttonEntityId, [[maybe_unused]] AZ::Vector2 position)
            {
                OnMasterVolumeToggle(ToggleDirection::Right);
            });
    }

    void UiSettingsComponent::Deactivate()
    {
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

    void UiSettingsComponent::OnGraphicsApiToggle(ToggleDirection toggleDirection)
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

        UiTextBus::Event(m_graphicsApiToggle.m_labelEntity, &UiTextInterface::SetText, valuesToLabels[graphicsApiIndex].second);

        MultiplayerSampleUserSettingsRequestBus::Broadcast(
            &MultiplayerSampleUserSettingsRequestBus::Events::SetGraphicsApi, valuesToLabels[graphicsApiIndex].first);

        MultiplayerSampleUserSettingsRequestBus::Broadcast(&MultiplayerSampleUserSettingsRequestBus::Events::Save);
    }

    void UiSettingsComponent::OnTextureQualityToggle(ToggleDirection toggleDirection)
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

        UiTextBus::Event(m_textureQualityToggle.m_labelEntity, &UiTextInterface::SetText, valuesToLabels[textureQualityIndex].second);

        MultiplayerSampleUserSettingsRequestBus::Broadcast(
            &MultiplayerSampleUserSettingsRequestBus::Events::SetTextureQuality, valuesToLabels[textureQualityIndex].first);

        MultiplayerSampleUserSettingsRequestBus::Broadcast(&MultiplayerSampleUserSettingsRequestBus::Events::Save);
    }

    void UiSettingsComponent::OnMasterVolumeToggle(ToggleDirection toggleDirection)
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

        UiTextBus::Event(m_masterVolumeToggle.m_labelEntity, &UiTextInterface::SetText, valuesToLabels[masterVolumeIndex].second);

        MultiplayerSampleUserSettingsRequestBus::Broadcast(
            &MultiplayerSampleUserSettingsRequestBus::Events::SetMasterVolume, valuesToLabels[masterVolumeIndex].first);

        MultiplayerSampleUserSettingsRequestBus::Broadcast(&MultiplayerSampleUserSettingsRequestBus::Events::Save);
    }

}
