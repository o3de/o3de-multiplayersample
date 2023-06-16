/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <Components/UI/UiGameLiftConnectWithPlayerSessionData.h>

#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>

#include <LyShine/Bus/UiButtonBus.h>
#include <LyShine/Bus/UiCursorBus.h>
#include <LyShine/Bus/UiElementBus.h>
#include <LyShine/Bus/UiTextBus.h>
#include <LyShine/Bus/UiTextInputBus.h>
#include <LyShine/Bus/UiInteractableBus.h>

#include <Multiplayer/Session/SessionRequests.h>
#include <Request/AWSGameLiftRequestBus.h>
#include <Request/AWSGameLiftSessionRequestBus.h>
#include <AzCore/Jobs/JobContext.h>
#include <AzCore/Jobs/JobFunction.h>
#include <Multiplayer/Session/ISessionHandlingRequests.h>


namespace MPSGameLift
{
    void UiGameLiftConnectWithPlayerSessionData::Reflect(AZ::ReflectContext* context)
    {
        if (const auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<UiGameLiftConnectWithPlayerSessionData, AZ::Component>()
                ->Version(1)
                ->Field("ConnectButton", &UiGameLiftConnectWithPlayerSessionData::m_connectButtonUi)
                ->Field("ExitButton", &UiGameLiftConnectWithPlayerSessionData::m_quitButtonUi)
                ->Field("PlayerSessionDataInputUi", &UiGameLiftConnectWithPlayerSessionData::m_playerSessionDataJsonInputUi)
                ->Field("AttemptConnectionBlockerUi", &UiGameLiftConnectWithPlayerSessionData::m_attemptConnectionBlockerUi)
                ->Field("ConnectToHostFailedUi", &UiGameLiftConnectWithPlayerSessionData::m_connectToHostFailedUi)
                ->Field("JsonParseFailTextUi", &UiGameLiftConnectWithPlayerSessionData::m_jsonParseFailTextUi)
                ;

            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                editContext->Class<UiGameLiftConnectWithPlayerSessionData>("UiGameLiftConnectWithPlayerSessionData", "Component to setup the start menu")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Category, "Multiplayer Sample UI")
                    ->Attribute(AZ::Edit::Attributes::Icon, "Icons/Components/Component_Placeholder.svg")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC_CE("CanvasUI"))

                    ->DataElement(AZ::Edit::UIHandlers::Default, &UiGameLiftConnectWithPlayerSessionData::m_connectButtonUi, "Connect Button", "The UI button hosting a game (only available for unified launchers which can run as a client-host).")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &UiGameLiftConnectWithPlayerSessionData::m_quitButtonUi, "Quit Button", "The UI button to quit the app.")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &UiGameLiftConnectWithPlayerSessionData::m_playerSessionDataJsonInputUi, "GameLift Player Session Text Input", "The UI text input providing the game session and player session id.")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &UiGameLiftConnectWithPlayerSessionData::m_attemptConnectionBlockerUi, "Attempt Connection Blocker", "Fullscreen UI for blocking user input while the client tries to connect.")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &UiGameLiftConnectWithPlayerSessionData::m_connectToHostFailedUi, "Connection To Host Failed", "UI to inform the user that connecting to the host failed.")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &UiGameLiftConnectWithPlayerSessionData::m_jsonParseFailTextUi, "Json Parse Fail Text", "UI to inform the user that current JSON string is missing some expected data.")
                    ;
            }
        }
    }

    void UiGameLiftConnectWithPlayerSessionData::Activate()
    {
        UiCursorBus::Broadcast(&UiCursorInterface::IncrementVisibleCounter);

        // Listen for button presses
        UiButtonBus::Event(m_quitButtonUi, &UiButtonInterface::SetOnClickCallback, [this](AZ::EntityId buttonEntityId, [[maybe_unused]] AZ::Vector2 position) { OnButtonClicked(buttonEntityId); });
        UiButtonBus::Event(m_connectButtonUi, &UiButtonInterface::SetOnClickCallback, [this](AZ::EntityId buttonEntityId, [[maybe_unused]] AZ::Vector2 position) { OnButtonClicked(buttonEntityId); });
        UiButtonBus::Event(m_connectToHostFailedUi, &UiButtonInterface::SetOnClickCallback, [this](AZ::EntityId buttonEntityId, [[maybe_unused]] AZ::Vector2 position) { OnButtonClicked(buttonEntityId); });
        UiTextInputBus::Event(m_playerSessionDataJsonInputUi, &UiTextInputInterface::SetOnChangeCallback, [this]([[maybe_unused]] AZ::EntityId entityId, const AZStd::string& gameLiftJsonString) { OnJSONChanged(gameLiftJsonString); });

        // Hide the attempting connection ui until the player tries to connect
        UiElementBus::Event(m_attemptConnectionBlockerUi, &UiElementInterface::SetIsEnabled, false);

        // Listen for disconnect events to know if connecting to the host server failed
        AZ::Interface<Multiplayer::IMultiplayer>::Get()->AddEndpointDisconnectedHandler(m_onConnectToHostFailed);

        UiElementBus::Event(m_jsonParseFailTextUi, &UiElementInterface::SetIsEnabled, true);
        UiTextBus::Event(m_jsonParseFailTextUi, &UiTextInterface::SetText, "");
        UiInteractableBus::Event(m_connectButtonUi, &UiInteractableInterface::SetIsHandlingEvents, false);
        OnJSONChanged("");
    }

    void UiGameLiftConnectWithPlayerSessionData::Deactivate()
    {
        m_onConnectToHostFailed.Disconnect();
        UiCursorBus::Broadcast(&UiCursorInterface::DecrementVisibleCounter);
    }

    void UiGameLiftConnectWithPlayerSessionData::OnJSONChanged(const AZStd::string& gameLiftJsonString)
    {
        // Disable the connect button until checking to make sure the user has provided the proper GameLift information in JSON format
        UiInteractableBus::Event(m_connectButtonUi, &UiInteractableInterface::SetIsHandlingEvents, false);

        if (gameLiftJsonString.empty())
        {
            UiTextBus::Event(m_jsonParseFailTextUi, &UiTextInterface::SetText, "Please provide GameLift player connection information in JSON format!");
            return;
        }
        
        // Parse GameLift JSON
        m_sessionConnectionConfig = {};
        m_region.clear();

        rapidjson::Document document;
        document.Parse(gameLiftJsonString.c_str());

        if (document.HasParseError())
        {
            UiTextBus::Event(m_jsonParseFailTextUi, &UiTextInterface::SetText, "Invalid JSON format!");
            return;
        }

        // Extract the AWS region from either a fleet arn or game session arn
        AZStd::string gameLiftArn;
        if (document.HasMember("GameSessionId"))
        {
            // Example game session format: "arn:aws:gamelift:us-west-2::gamesession<id>"
            const rapidjson::Value& gameSessionId = document["GameSessionId"];
            gameLiftArn = gameSessionId.GetString();
        }
        else if (document.HasMember("FleetArn"))
        {
            // Example fleet arn format: "arn:aws:gamelift:us-west-2:353687041169:fleet<id>"
            const rapidjson::Value& fleetArn = document["FleetArn"];
            gameLiftArn = fleetArn.GetString();
        }

        m_region = AWSCore::Util::ExtractRegion(gameLiftArn);
        if (m_region.empty())
        {
            UiTextBus::Event(m_jsonParseFailTextUi, &UiTextInterface::SetText, "Failed to extract AWS region. Provide either a valid GameSessionId or FleetArn!");
            return;
        }

        // Alert the user if any other information is missing from the JSON they provided
        if (!document.HasMember("PlayerSessionId"))
        {
            UiTextBus::Event(m_jsonParseFailTextUi, &UiTextInterface::SetText, "Missing PlayerSessionId!");
            return;
        }

        if (!document.HasMember("IpAddress") && !document.HasMember("DnsName"))
        {
            UiTextBus::Event(m_jsonParseFailTextUi, &UiTextInterface::SetText, "Must provide either an IpAddress or DnsName!");
            return;
        }

        if (!document.HasMember("Port"))
        {
            UiTextBus::Event(m_jsonParseFailTextUi, &UiTextInterface::SetText, "Missing Port!");
            return;
        }

        const rapidjson::Value& port = document["Port"];
        if (!port.IsUint())
        {
            UiTextBus::Event(m_jsonParseFailTextUi, &UiTextInterface::SetText, "Invalid Port!");
            return;
        }

        // Fill out SessionConnectionConfig and try connecting to host
        if (document.HasMember("IpAddress"))
        {
            const rapidjson::Value& ipAddress = document["IpAddress"];
            m_sessionConnectionConfig.m_ipAddress = ipAddress.GetString();
        }

        if (document.HasMember("DnsName"))
        {
            const rapidjson::Value& dnsName = document["DnsName"];
            m_sessionConnectionConfig.m_dnsName = dnsName.GetString();
        }

        const rapidjson::Value& playerSessionId = document["PlayerSessionId"];

        m_sessionConnectionConfig.m_port = aznumeric_cast<uint16_t>(port.GetUint());
        m_sessionConnectionConfig.m_playerSessionId = playerSessionId.GetString();

        UiTextBus::Event(m_jsonParseFailTextUi, &UiTextInterface::SetText, "");
        UiInteractableBus::Event(m_connectButtonUi, &UiInteractableInterface::SetIsHandlingEvents, true);
    }

    void UiGameLiftConnectWithPlayerSessionData::OnButtonClicked(AZ::EntityId buttonEntityId) const
    {
        const auto console = AZ::Interface<AZ::IConsole>::Get();
        if (!console)
        {
            AZ_Assert(false, "UiGameLiftConnectWithPlayerSessionData attempting to use console commands before AZ::Console is available.");
            return;
        }

        if (buttonEntityId == m_quitButtonUi)
        {
            console->PerformCommand("quit");
            return;
        }
        
        if (buttonEntityId == m_connectButtonUi)
        {
            // Enable blocker ui while we attempt connection
            UiElementBus::Event(m_attemptConnectionBlockerUi, &UiElementInterface::SetIsEnabled, true);

            // Enable GameLift and connect to host
            AWSGameLift::AWSGameLiftRequestBus::Broadcast(&AWSGameLift::AWSGameLiftRequestBus::Events::ConfigureGameLiftClient, m_region);
            if (auto clientRequestHandler = AZ::Interface<Multiplayer::ISessionHandlingClientRequests>::Get())
            {
                clientRequestHandler->RequestPlayerJoinSession(m_sessionConnectionConfig);
            }
            else
            {
                AZ_Assert(false, "UiGameLiftConnectWithPlayerSessionData failed to connect because there's no ISessionHandlingClientRequests registered. " 
                    "Please update code to ensure an ISessionHandlingClientRequests has been created before trying to connect this client to a host!");
            }
        }
        
        if (buttonEntityId == m_connectToHostFailedUi)
        {
            // Player acknowledged connection failed. Close the warning popup.
            UiElementBus::Event(m_connectToHostFailedUi, &UiElementInterface::SetIsEnabled, false);
        }
    }

    void UiGameLiftConnectWithPlayerSessionData::OnConnectToHostFailed()
    {
        UiElementBus::Event(m_attemptConnectionBlockerUi, &UiElementInterface::SetIsEnabled, false);
        UiElementBus::Event(m_connectToHostFailedUi, &UiElementInterface::SetIsEnabled, true);
    }    
} // namespace MultiplayerSample
