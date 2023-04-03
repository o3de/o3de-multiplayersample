/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 * 
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include "MultiplayerSampleAWSGameLiftClientSystemComponent.h"

#include <AzCore/Serialization/SerializeContext.h>
#include <Request/AWSGameLiftRequestBus.h>
#include <Request/AWSGameLiftSessionRequestBus.h>
#include <Request/AWSGameLiftJoinSessionRequest.h>

namespace MultiplayerSample
{
    void MultiplayerSampleAWSGameLiftClientSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serialize = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serialize->Class<MultiplayerSampleAWSGameLiftClientSystemComponent, AZ::Component>()
                ->Version(0)
                ;
        }
    }

    void MultiplayerSampleAWSGameLiftClientSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("MultiplayerSampleAWSGameLiftClientService"));
    }

    void MultiplayerSampleAWSGameLiftClientSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("MultiplayerSampleAWSGameLiftClientService"));
    }

    void MultiplayerSampleAWSGameLiftClientSystemComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        required.push_back(AZ_CRC_CE("AWSGameLiftClientService"));
    }

    void MultiplayerSampleAWSGameLiftClientSystemComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
    }

    void MultiplayerSampleAWSGameLiftClientSystemComponent::Init()
    {
        m_playerId = AZ::Uuid().Create().ToString<AZStd::string>();
    }

    void MultiplayerSampleAWSGameLiftClientSystemComponent::Activate()
    {
        AWSGameLift::AWSGameLiftRequestBus::Broadcast(&AWSGameLift::AWSGameLiftRequestBus::Events::ConfigureGameLiftClient, "");
    } 
    void MultiplayerSampleAWSGameLiftClientSystemComponent::Deactivate()
    {
    }

    void MultiplayerSampleAWSGameLiftClientSystemComponent::JoinSession(const AZ::ConsoleCommandContainer& consoleFunctionParameters)
    {
        if (consoleFunctionParameters.size() != 1)
        {
            AZ_Error("MPSGameLiftClientSystemComponent", false, "Invalid console command. Use JoinSession <game-session-id>");
            return;
        }

        JoinSessionInternal(consoleFunctionParameters[0], m_playerId);
    }

    void MultiplayerSampleAWSGameLiftClientSystemComponent::JoinSessionInternal(const AZStd::string& sessionId, const AZStd::string& playerId)
    {
        AWSGameLift::AWSGameLiftJoinSessionRequest request;
        request.m_sessionId = sessionId;
        request.m_playerId = playerId;

        AWSGameLift::AWSGameLiftSessionAsyncRequestBus::Broadcast(
            &AWSGameLift::AWSGameLiftSessionAsyncRequestBus::Events::JoinSessionAsync, request);
    }

}
