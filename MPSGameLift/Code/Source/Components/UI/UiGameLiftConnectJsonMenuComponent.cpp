/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <Components/UI/UiGameLiftConnectJsonMenuComponent.h>

#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>

#include <LyShine/Bus/UiButtonBus.h>
#include <LyShine/Bus/UiCursorBus.h>
#include <LyShine/Bus/UiElementBus.h>
#include <LyShine/Bus/UiTextInputBus.h>

#include <Multiplayer/Session/SessionRequests.h>

#include <Request/AWSGameLiftJoinSessionRequest.h>
#include <Request/AWSGameLiftSessionRequestBus.h>

#pragma optimize("",off)
namespace MPSGameLift
{
    void UiGameLiftConnectJsonMenuComponent::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<UiGameLiftConnectJsonMenuComponent, AZ::Component>()
                ->Version(1)
                ->Field("ConnectButton", &UiGameLiftConnectJsonMenuComponent::m_connectButtonUi)
                ->Field("ExitButton", &UiGameLiftConnectJsonMenuComponent::m_exitButtonUi)
                ->Field("IPAddressTextInput", &UiGameLiftConnectJsonMenuComponent::m_ipAddressTextInputUi)
                ->Field("AttemptConnectionBlockerUi", &UiGameLiftConnectJsonMenuComponent::m_attemptConnectionBlockerUi)
                ->Field("ConnectToHostFailedUi", &UiGameLiftConnectJsonMenuComponent::m_connectToHostFailedUi)
                ;

            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                editContext->Class<UiGameLiftConnectJsonMenuComponent>("UiGameLiftConnectJsonMenuComponent", "Component to setup the start menu")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Category, "Multiplayer Sample UI")
                    ->Attribute(AZ::Edit::Attributes::Icon, "Icons/Components/Component_Placeholder.svg")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC_CE("CanvasUI"))

                    ->DataElement(AZ::Edit::UIHandlers::Default, &UiGameLiftConnectJsonMenuComponent::m_connectButtonUi, "Host Button", "The ui button hosting a game (only available for unified launchers which can run as a client-host).")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &UiGameLiftConnectJsonMenuComponent::m_exitButtonUi, "Exit Button", "The ui button to quit the app.")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &UiGameLiftConnectJsonMenuComponent::m_ipAddressTextInputUi, "IP Address TextInput", "The ui text input providing the ip address to join.")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &UiGameLiftConnectJsonMenuComponent::m_attemptConnectionBlockerUi, "Attempt Connection Blocker", "Fullscreen ui for blocking user input while the client tries to connect.")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &UiGameLiftConnectJsonMenuComponent::m_connectToHostFailedUi, "Connection To Host Failed", "Ui to inform the user that connecting to the host failed.")
                    ;
            }
        }
    }

    void UiGameLiftConnectJsonMenuComponent::Activate()
    {
        UiCursorBus::Broadcast(&UiCursorInterface::IncrementVisibleCounter);

        // Listen for button presses
        UiButtonBus::Event(m_exitButtonUi, &UiButtonInterface::SetOnClickCallback, [this](AZ::EntityId buttonEntityId, [[maybe_unused]] AZ::Vector2 position) {OnButtonClicked(buttonEntityId); });
        UiButtonBus::Event(m_connectButtonUi, &UiButtonInterface::SetOnClickCallback, [this](AZ::EntityId buttonEntityId, [[maybe_unused]] AZ::Vector2 position) {OnButtonClicked(buttonEntityId); });
        UiButtonBus::Event(m_connectToHostFailedUi, &UiButtonInterface::SetOnClickCallback, [this](AZ::EntityId buttonEntityId, [[maybe_unused]] AZ::Vector2 position) {OnButtonClicked(buttonEntityId); });

        // Hide the host button if this app can't host
        #if !AZ_TRAIT_SERVER
            UiElementBus::Event(m_connectButtonUi, &UiElementInterface::SetIsEnabled, false);
        #endif

        // Hide the attempting connection ui until the player tries to connect
        UiElementBus::Event(m_attemptConnectionBlockerUi, &UiElementInterface::SetIsEnabled, false);

        // Listen for disconnect events to know if connecting to the host server failed
        AZ::Interface<Multiplayer::IMultiplayer>::Get()->AddEndpointDisconnectedHandler(m_onConnectToHostFailed);

        // Set default text of the ip-address input textbox based on 'cl_serverAddr'
        if (const auto console = AZ::Interface<AZ::IConsole>::Get())
        {
            AZ::CVarFixedString serverAddress;
            if (console->GetCvarValue("cl_serverAddr", serverAddress) == AZ::GetValueResult::Success)
            {
                UiTextInputBus::Event(m_ipAddressTextInputUi, &UiTextInputInterface::SetText, serverAddress.c_str());
            }
            else
            {
                AZ_Warning("UiGameLiftConnectJsonMenuComponent", false, "Could not access cl_serveraddr cvar, so the ip-address input textfield won't be filled out. Please update UiGameLiftConnectJsonMenuComponent.cpp to check for a valid cvar!")
            }
        }
    }

    void UiGameLiftConnectJsonMenuComponent::Deactivate()
    {
        m_onConnectToHostFailed.Disconnect();
        UiCursorBus::Broadcast(&UiCursorInterface::DecrementVisibleCounter);
    }

    void UiGameLiftConnectJsonMenuComponent::OnButtonClicked(AZ::EntityId buttonEntityId) const
    {
        const auto console = AZ::Interface<AZ::IConsole>::Get();
        if (!console)
        {
            AZ_Assert(false, "UiGameLiftConnectJsonMenuComponent attempting to use console commands before AZ::Console is available.");
            return;
        }

        if (buttonEntityId == m_exitButtonUi)
        {
            console->PerformCommand("quit");
        }
        else if (buttonEntityId == m_connectButtonUi)
        {
            // Enable blocker ui while we attempt connection
            
            // Parse GameLift JSON
            AZStd::string gameLiftJsonString;
            UiTextInputBus::EventResult(gameLiftJsonString, m_ipAddressTextInputUi, &UiTextInputInterface::GetText);

            AWSGameLift::AWSGameLiftJoinSessionRequest request;
            rapidjson::Document document;
            document.Parse(gameLiftJsonString.c_str());

            const rapidjson::Value& gameSessionId = document["GameSessionId"];
            if (gameSessionId.IsString())
            {
                request.m_sessionId = gameSessionId.GetString();
            }

            const rapidjson::Value& playerSessionId = document["PlayerSessionId"];
            if (playerSessionId.IsString())
            {
                request.m_playerId = playerSessionId.GetString();
            }

            AWSGameLift::AWSGameLiftSessionAsyncRequestBus::Broadcast(
                &AWSGameLift::AWSGameLiftSessionAsyncRequestBus::Events::JoinSessionAsync, request);
        }
        else if (buttonEntityId == m_connectToHostFailedUi)
        {
            // Player acknowledged connection failed. Close the warning popup.
            UiElementBus::Event(m_connectToHostFailedUi, &UiElementInterface::SetIsEnabled, false);
        }
    }

    void UiGameLiftConnectJsonMenuComponent::OnConnectToHostFailed()
    {
        UiElementBus::Event(m_attemptConnectionBlockerUi, &UiElementInterface::SetIsEnabled, false);
        UiElementBus::Event(m_connectToHostFailedUi, &UiElementInterface::SetIsEnabled, true);
    }

} // namespace MultiplayerSample
#pragma optimize("",on)
