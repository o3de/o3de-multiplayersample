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
    }

    void UiGameLiftFlexMatchConnect::Deactivate()
    {
        m_onConnectToHostFailed.Disconnect();
        UiCursorBus::Broadcast(&UiCursorInterface::DecrementVisibleCounter);
    }

    void UiGameLiftFlexMatchConnect::OnButtonClicked(AZ::EntityId buttonEntityId) const
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

            AZ::Interface<IRegionalLatencyFinder>::Get()->RequestLatencies();
        }
        
        if (buttonEntityId == m_connectToHostFailedUi)
        {
            // Player acknowledged connection failed. Close the warning popup.
            UiElementBus::Event(m_connectToHostFailedUi, &UiElementInterface::SetIsEnabled, false);
        }
    }

    void UiGameLiftFlexMatchConnect::OnRequestLatenciesComplete(const RegionalLatencies& regionLatencies)
    {
        UiTextBus::Event(m_matchmakingStatusTextUi, &UiTextInterface::SetText, "Latencies found...");

        AZStd::string latencyPrint;
        for (const auto latency : regionLatencies)
        {
            latencyPrint += AZStd::string::format("%s: %ims\n", latency.first.c_str(), static_cast<uint32_t>(latency.second.count()));
        }

        UiTextBus::Event(m_matchmakingStatusTextUi, &UiTextInterface::SetText, latencyPrint.c_str());

    }

    void UiGameLiftFlexMatchConnect::OnConnectToHostFailed()
    {
        UiElementBus::Event(m_attemptConnectionBlockerUi, &UiElementInterface::SetIsEnabled, false);
        UiElementBus::Event(m_connectToHostFailedUi, &UiElementInterface::SetIsEnabled, true);
    }    
} // namespace MultiplayerSample
