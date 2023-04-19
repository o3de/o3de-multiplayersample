/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/Component/Component.h>
#include <IAudioSystem.h>

namespace MultiplayerSample
{
    class BackgroundMusicComponent
        : public AZ::Component
        , protected Audio::AudioTriggerNotificationBus::Handler
    {
    public:
        AZ_COMPONENT(MultiplayerSample::BackgroundMusicComponent, "{FA774915-3CDD-4370-B7C9-8F891A006973}");

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& services);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& services);

        void Activate() override;
        void Deactivate() override;

    protected:
        void ReportTriggerFinished(Audio::TAudioControlID triggerId) override;

    private:
        size_t m_trackIndex = 0;
        bool m_shuffle = false;
        AZStd::vector<AZStd::string> m_playlist;
        Audio::IAudioSystem* m_audioSystem = nullptr;
        Audio::TATLIDType m_currentTrackTriggerId = INVALID_AUDIO_CONTROL_ID;
    };
}
