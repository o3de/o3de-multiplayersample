/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <Atom/RHI/Factory.h>
#include <AzCore/Console/IConsole.h>
#include <AzCore/Interface/Interface.h>
#include <AzCore/Serialization/EditContext.h>
#include <IAudioSystem.h>
#include <LyShine/Bus/UiButtonBus.h>
#include <LyShine/Bus/UiElementBus.h>
#include <LyShine/Bus/UiTextBus.h>
#include <Source/Components/UI/UiSettingsComponent.h>

namespace MultiplayerSample
{
    void MpsSettings::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<MpsSettings>()
                ->Version(0)
                ->Field("GraphicsApi", &MpsSettings::m_atomApiType)
                ->Field("MasterVolume", &MpsSettings::m_masterVolume)
                ->Field("TextureQuality", &MpsSettings::m_streamingImageMipBias)
                ;
        }
    }

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
        MpsSettings::Reflect(context);
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
        // Initialize our user settings 

        // Initialize the current streaming image mip bias setting.
        if (AZ::IConsole* console = AZ::Interface<AZ::IConsole>::Get(); console)
        {
            int16_t mipBias = 0;
            console->GetCvarValue("r_streamingImageMipBias", mipBias);
            m_settings.m_streamingImageMipBias = aznumeric_cast<uint8_t>(mipBias);
        }

        // Initialize the graphics API type
        m_settings.m_atomApiType = AZ::RHI::Factory::Get().GetAPIUniqueIndex();

        // There's currently no way to initialize the master volume, this doesn't seem to be fetchable anywhere.

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

    void UiSettingsComponent::OnGraphicsApiToggle(ToggleDirection toggleDirection)
    {
        // This list is expected to match the values in AZ::RHI::ApiIndex.
        const char* labels[] =
        {
            "Null",
            "DirectX 12",
            "Vulkan",
            "Metal"
        };

        const size_t NumLabels = AZ_ARRAY_SIZE(labels);

        if (toggleDirection != ToggleDirection::None)
        {
            m_settings.m_atomApiType = (toggleDirection == ToggleDirection::Right)
                ? (m_settings.m_atomApiType + 1) % NumLabels
                : (m_settings.m_atomApiType + (NumLabels - 1)) % NumLabels
                ;
        }

        UiTextBus::Event(m_graphicsApiToggle.m_labelEntity, &UiTextInterface::SetText, labels[m_settings.m_atomApiType]);
    }

    void UiSettingsComponent::OnTextureQualityToggle(ToggleDirection toggleDirection)
    {
        const char* labels[] =
        {
            "Ultra (4K)",
            "High (2K)",
            "Medium (1K)",
            "Low (512)",
            "Very Low (256)",
            "Extremely Low (128)",
            "Rock Bottom (64)"
        };

        const size_t NumLabels = AZ_ARRAY_SIZE(labels);

        if (toggleDirection != ToggleDirection::None)
        {
            // As we go from left to right on our settings, we want our textureQuality number to go from 6 down to 0
            // because smaller mip bias numbers mean higher-resolution textures.
            m_settings.m_streamingImageMipBias = (toggleDirection == ToggleDirection::Right)
                ? (m_settings.m_streamingImageMipBias + (NumLabels - 1)) % NumLabels
                : (m_settings.m_streamingImageMipBias + 1) % NumLabels
                ;
        }

        AZ::IConsole* console = AZ::Interface<AZ::IConsole>::Get();
        if (console)
        {
            AZ::CVarFixedString commandString = AZ::CVarFixedString::format("r_streamingImageMipBias %" PRId16, m_settings.m_streamingImageMipBias);
            console->PerformCommand(commandString.c_str());
        }

        UiTextBus::Event(m_textureQualityToggle.m_labelEntity, &UiTextInterface::SetText, labels[m_settings.m_streamingImageMipBias]);
    }

    void UiSettingsComponent::OnMasterVolumeToggle(ToggleDirection toggleDirection)
    {
        if (toggleDirection != ToggleDirection::None)
        {
            m_settings.m_masterVolume = (toggleDirection == ToggleDirection::Right)
                ? (m_settings.m_masterVolume + 10) % 110
                : (m_settings.m_masterVolume + 100) % 110
                ;
        }

        auto audioSystem = AZ::Interface<Audio::IAudioSystem>::Get();
        if (audioSystem)
        {
            Audio::TAudioObjectID rtpcId = audioSystem->GetAudioRtpcID("Volume_Master");

            if (rtpcId != INVALID_AUDIO_CONTROL_ID)
            {
                Audio::ObjectRequest::SetParameterValue setParameter;
                setParameter.m_audioObjectId = INVALID_AUDIO_OBJECT_ID;
                setParameter.m_parameterId = rtpcId;
                setParameter.m_value = m_settings.m_masterVolume / 100.0f;
                AZ::Interface<Audio::IAudioSystem>::Get()->PushRequest(AZStd::move(setParameter));
            }
        }

        UiTextBus::Event(m_masterVolumeToggle.m_labelEntity, &UiTextInterface::SetText, AZStd::string::format("%d", m_settings.m_masterVolume));
    }

}
