/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <Source/Components/BackgroundMusicComponent.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <LmbrCentral/Audio/AudioProxyComponentBus.h>
#include <AzCore/Console/ILogger.h>

namespace MultiplayerSample
{
    void BackgroundMusicComponent::Reflect(AZ::ReflectContext* context)
    {
        AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context);
        if (serializeContext)
        {
            serializeContext->Class<BackgroundMusicComponent, AZ::Component>()
                ->Field("Playlist", &BackgroundMusicComponent::m_playlist)
                ->Version(1);

            AZ::EditContext* editContext = serializeContext->GetEditContext();
            if (editContext)
            {
                editContext->Class<BackgroundMusicComponent>("BackgroundMusicComponent", "Plays a sequence of background music tracks in sequence")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Category, "MultiplayerSample")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC_CE("Game"))
                    ->DataElement(AZ::Edit::UIHandlers::Default, &BackgroundMusicComponent::m_playlist, "Playlist", "The set of background music audio triggers to play");
            }
        }
    }

    void BackgroundMusicComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& services)
    {
        services.push_back(AZ_CRC_CE("BackgroundMusicComponent"));
    }

    void BackgroundMusicComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        required.push_back(AZ_CRC("AudioProxyService", 0x7da4c79c));
    }

    void BackgroundMusicComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& services)
    {
        services.push_back(AZ_CRC_CE("BackgroundMusicComponent"));
    }

    void BackgroundMusicComponent::Activate()
    {
#if AZ_TRAIT_CLIENT
        m_audioSystem = AZ::Interface<Audio::IAudioSystem>::Get();

        if (m_audioSystem != nullptr)
        {
            m_trackIndex = size_t(-1);
            ReportTriggerFinished(INVALID_AUDIO_CONTROL_ID);
        }
#endif

        Audio::AudioTriggerNotificationBus::Handler::BusConnect(Audio::TriggerNotificationIdType{ GetEntityId() });
    }

    void BackgroundMusicComponent::Deactivate()
    {
        Audio::AudioTriggerNotificationBus::Handler::BusDisconnect(Audio::TriggerNotificationIdType{ GetEntityId() });
    }

    void BackgroundMusicComponent::ReportTriggerFinished([[maybe_unused]] Audio::TAudioControlID triggerId)
    {
        if (m_playlist.empty())
        {
            return;
        }

#if AZ_TRAIT_CLIENT
        m_trackIndex = ++m_trackIndex % m_playlist.size();

        if (m_audioSystem != nullptr)
        {
            const AZStd::string& trackName = m_playlist[m_trackIndex];
            m_currentTrackTriggerId = m_audioSystem->GetAudioTriggerID(trackName.c_str());

            if (m_currentTrackTriggerId != INVALID_AUDIO_CONTROL_ID)
            {
                LmbrCentral::AudioProxyComponentRequestBus::Event(
                    GetEntityId(),
                    &LmbrCentral::AudioProxyComponentRequests::ExecuteTrigger,
                    m_currentTrackTriggerId);
            }
        }
#endif
    }
}
