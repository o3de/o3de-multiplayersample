/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 * 
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/Component/Component.h>
#include <AzCore/Console/IConsole.h>

namespace MultiplayerSample
{
    class MultiplayerSampleAWSGameLiftClientSystemComponent
        : public AZ::Component
    {
    public:
        AZ_COMPONENT(MultiplayerSampleAWSGameLiftClientSystemComponent, "{09b3eb68-8a50-485b-a4b0-38c5626ca78e}");

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

    protected:
        // AZ::Component interface implementation
        void Init() override;
        void Activate() override;
        void Deactivate() override;

        void JoinSession(const AZ::ConsoleCommandContainer& consoleFunctionParameters);
        AZ_CONSOLEFUNC(MultiplayerSampleAWSGameLiftClientSystemComponent, JoinSession, AZ::ConsoleFunctorFlags::DontReplicate, "Join an existing game session");

    private:
        void JoinSessionInternal(const AZStd::string& sessionId, const AZStd::string& playerId);

        AZStd::string m_playerId; // Unique identifier for the current player inside the game session
    };
}
