/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <Components/UI/UiGameLiftFlexMatchConnect.h>

#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>

#include <LyShine/Bus/UiButtonBus.h>
#include <LyShine/Bus/UiCursorBus.h>
#include <LyShine/Bus/UiElementBus.h>
#include <LyShine/Bus/UiTextBus.h>

#include <Multiplayer/Session/ISessionHandlingRequests.h>


namespace MPSGameLift
{
    void UiGameLiftFlexMatchConnect::Reflect(AZ::ReflectContext* context)
    {
        if (const auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<UiGameLiftFlexMatchConnect, AZ::Component>()
                ->Version(1)
                ->Field("ConnectButton", &UiGameLiftFlexMatchConnect::m_connectButtonUi)
                ->Field("MatchmakingStatusTextUi", &UiGameLiftFlexMatchConnect::m_matchmakingStatusTextUi)
                ->Field("ExitButton", &UiGameLiftFlexMatchConnect::m_quitButtonUi)
                ->Field("AttemptConnectionBlockerUi", &UiGameLiftFlexMatchConnect::m_attemptConnectionBlockerUi)
                ->Field("ConnectToHostFailedUi", &UiGameLiftFlexMatchConnect::m_connectToHostFailedUi)
                ;

            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                editContext->Class<UiGameLiftFlexMatchConnect>("UiGameLiftFlexMatchConnect", "Component to setup the start menu")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Category, "Multiplayer Sample UI")
                    ->Attribute(AZ::Edit::Attributes::Icon, "Icons/Components/Component_Placeholder.svg")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC_CE("CanvasUI"))

                    ->DataElement(AZ::Edit::UIHandlers::Default, &UiGameLiftFlexMatchConnect::m_connectButtonUi, "Connect Button", "The UI button hosting a game (only available for unified launchers which can run as a client-host).")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &UiGameLiftFlexMatchConnect::m_matchmakingStatusTextUi, "Matchmaking Status Text", "The UI text to display the progress of finding a match after the player presses the connect button.")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &UiGameLiftFlexMatchConnect::m_quitButtonUi, "Quit Button", "The UI button to quit the app.")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &UiGameLiftFlexMatchConnect::m_attemptConnectionBlockerUi, "Attempt Connection Blocker", "Fullscreen UI for blocking user input while the client tries to connect.")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &UiGameLiftFlexMatchConnect::m_connectToHostFailedUi, "Connection To Host Failed", "UI to inform the user that connecting to the host failed.")
                    ;
            }
        }
    }

    void UiGameLiftFlexMatchConnect::Activate()
    {
        UiCursorBus::Broadcast(&UiCursorInterface::IncrementVisibleCounter);

        // Listen for button presses
        UiButtonBus::Event(m_quitButtonUi, &UiButtonInterface::SetOnClickCallback, [this](AZ::EntityId buttonEntityId, [[maybe_unused]] AZ::Vector2 position) { OnButtonClicked(buttonEntityId); });
        UiButtonBus::Event(m_connectButtonUi, &UiButtonInterface::SetOnClickCallback, [this](AZ::EntityId buttonEntityId, [[maybe_unused]] AZ::Vector2 position) { OnButtonClicked(buttonEntityId); });
        UiButtonBus::Event(m_connectToHostFailedUi, &UiButtonInterface::SetOnClickCallback, [this](AZ::EntityId buttonEntityId, [[maybe_unused]] AZ::Vector2 position) { OnButtonClicked(buttonEntityId); });

        // Hide the attempting connection ui until the player tries to connect
        UiElementBus::Event(m_attemptConnectionBlockerUi, &UiElementInterface::SetIsEnabled, false);

        // Listen for disconnect events to know if connecting to the host server failed
        AZ::Interface<Multiplayer::IMultiplayer>::Get()->AddEndpointDisconnectedHandler(m_onConnectToHostFailed);
        AZ::Interface<IRegionalLatencyFinder>::Get()->AddRequestLatenciesCompleteEventHandler(m_requestLatenciesComplete);

        // Listen for auth events
        AWSClientAuth::AWSCognitoAuthorizationNotificationBus::Handler::BusConnect();

        // Listen for matchmaking events
        AZ::Interface<IMatchmaking>::Get()->AddMatchmakingFailedEventHandler(m_onMatchmakingFailed);
        AZ::Interface<IMatchmaking>::Get()->AddMatchmakingSuccessEventHandler(m_onMatchmakingSuccess);
        AZ::Interface<IMatchmaking>::Get()->AddMatchmakingTicketReceivedEventHandler(m_onMatchmakingTicketReceived);
    }

    void UiGameLiftFlexMatchConnect::Deactivate()
    {
        AWSClientAuth::AWSCognitoAuthorizationNotificationBus::Handler::BusDisconnect();
        m_onConnectToHostFailed.Disconnect();
        UiCursorBus::Broadcast(&UiCursorInterface::DecrementVisibleCounter);
    }

    void UiGameLiftFlexMatchConnect::OnButtonClicked(AZ::EntityId buttonEntityId)
    {
        const auto console = AZ::Interface<AZ::IConsole>::Get();
        if (!console)
        {
            AZ_Assert(false, "UiGameLiftFlexMatchConnect attempting to use console commands before AZ::Console is available.");
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
            UiTextBus::Event(m_matchmakingStatusTextUi, &UiTextInterface::SetText, "Searching for match...");

            bool clientAuthInitialized = false;
            AWSClientAuth::AWSCognitoAuthorizationRequestBus::BroadcastResult(clientAuthInitialized, &AWSClientAuth::IAWSCognitoAuthorizationRequests::Initialize);

            if (clientAuthInitialized)
            {
                m_statusUpdates.push_back(StatusPlayerAuthInitSuccess);
            }
            else
            {
                PushStatusFail(StatusPlayerAuthInitFailed);
                return;
            }
            
            AWSClientAuth::AWSCognitoAuthorizationRequestBus::Broadcast(&AWSClientAuth::IAWSCognitoAuthorizationRequests::RequestAWSCredentialsAsync);
        }
        
        if (buttonEntityId == m_connectToHostFailedUi)
        {
            // Player acknowledged connection failed. Close the warning popup.
            UiElementBus::Event(m_connectToHostFailedUi, &UiElementInterface::SetIsEnabled, false);
            UiElementBus::Event(m_attemptConnectionBlockerUi, &UiElementInterface::SetIsEnabled, false);
            m_statusUpdates.clear();
        }
    }

    void UiGameLiftFlexMatchConnect::OnRequestLatenciesComplete(const RegionalLatencies& regionalLatencies)
    {
        if (regionalLatencies.empty())
        {
            PushStatusFail(StatusLatencyRequestFailed);
            return;
        }

        // Tell player server endpoints were reached and display the round-trip-time...
        AZStd::string latencyPrint;
        for (const auto& latency : regionalLatencies)
        {
            latencyPrint += AZStd::string::format("%s: %ims\n", latency.first.c_str(), static_cast<uint32_t>(latency.second.count()));
        }
        
        ReplaceStatusUpdate(AZStd::string::format(StatusLatencyRequestSuccess, latencyPrint.c_str()));

        // Start matchmaking
        AZ::Interface<IMatchmaking>::Get()->RequestMatch(regionalLatencies);
    }

    void UiGameLiftFlexMatchConnect::OnConnectToHostFailed()
    {
        UiElementBus::Event(m_attemptConnectionBlockerUi, &UiElementInterface::SetIsEnabled, false);
        UiElementBus::Event(m_connectToHostFailedUi, &UiElementInterface::SetIsEnabled, true);
    }

    void UiGameLiftFlexMatchConnect::OnRequestAWSCredentialsSuccess([[maybe_unused]] const AWSClientAuth::ClientAuthAWSCredentials& awsCredentials)
    {
        ReplaceStatusUpdate(StatusAnonPlayerCredentialsReceived);
        PushStatusUpdate(StatusRequestingServerRegionLatencies);
        AZ::Interface<IRegionalLatencyFinder>::Get()->RequestLatencies();
    }

    void UiGameLiftFlexMatchConnect::OnRequestAWSCredentialsFail([[maybe_unused]] const AZStd::string& error)
    {
        PushStatusFail(StatusAnonPlayerCredentialsFailed);
    }

    void UiGameLiftFlexMatchConnect::PushStatusUpdate(const AZStd::string& statusUpdate)
    {
        m_statusUpdates.push_back(statusUpdate);
        RenderStatusText();
    }
    
    void UiGameLiftFlexMatchConnect::ReplaceStatusUpdate(const AZStd::string& statusUpdate)
    {
        m_statusUpdates.pop_back();
        m_statusUpdates.push_back(statusUpdate);
        RenderStatusText();
    }
    
    void UiGameLiftFlexMatchConnect::PushStatusFail(const AZStd::string& reason)
    {
        // Display the latest status update in red
        const AZStd::string markupRedFont = "<font color = \"#ff0000\">";
        PushStatusUpdate(markupRedFont + reason + "</font>");
        
        // Enable the matchmaking failed popup, and allow the user to close the popup, and try again.
        UiElementBus::Event(m_connectToHostFailedUi, &UiElementInterface::SetIsEnabled, true);
    }

    void UiGameLiftFlexMatchConnect::RenderStatusText()
    {
        // Combine each all the status updates into a single list and render to UI text.
        AZStd::string statusTextbox;
        for (const auto& status : m_statusUpdates)
        {
            statusTextbox += status + "\n";
        }

        UiTextBus::Event(m_matchmakingStatusTextUi, &UiTextInterface::SetText, statusTextbox);
    }
} // namespace MultiplayerSample
