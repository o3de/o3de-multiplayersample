/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <Source/Components/UI/UiStartMenuComponent.h>

#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>

#include <LyShine/Bus/UiButtonBus.h>
#include <LyShine/Bus/UiCursorBus.h>
#include <LyShine/Bus/UiElementBus.h>
#include <LyShine/Bus/UiTextInputBus.h>

namespace MultiplayerSample
{
    void UiStartMenuComponent::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<UiStartMenuComponent, AZ::Component>()
                ->Version(1)
                ->Field("HostButton", &UiStartMenuComponent::m_hostButtonUi)
                ->Field("JoinButton", &UiStartMenuComponent::m_joinButtonUi)
                ->Field("ExitButton", &UiStartMenuComponent::m_exitButtonUi)
                ->Field("IPAddressTextInput", &UiStartMenuComponent::m_ipAddressTextInputUi)
                ->Field("AttemptConnectionBlockerUi", &UiStartMenuComponent::m_attemptConnectionBlockerUi)
                ->Field("ConnectToHostFailedUi", &UiStartMenuComponent::m_connectToHostFailedUi)
                ;

            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                editContext->Class<UiStartMenuComponent>("UiStartMenuComponent", "Component to setup the start menu")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Category, "Multiplayer Sample UI")
                    ->Attribute(AZ::Edit::Attributes::Icon, "Icons/Components/Component_Placeholder.svg")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC_CE("CanvasUI"))

                    ->DataElement(AZ::Edit::UIHandlers::Default, &UiStartMenuComponent::m_hostButtonUi, "Host Button", "The ui button hosting a game (only available for unified launchers which can run as a client-host).")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &UiStartMenuComponent::m_joinButtonUi, "Join Button", "The ui button joining a multiplayer game based on the provided ip address.")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &UiStartMenuComponent::m_exitButtonUi, "Exit Button", "The ui button to quit the app.")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &UiStartMenuComponent::m_ipAddressTextInputUi, "IP Address TextInput", "The ui text input providing the ip address to join.")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &UiStartMenuComponent::m_attemptConnectionBlockerUi, "Attempt Connection Blocker", "Fullscreen ui for blocking user input while the client tries to connect.")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &UiStartMenuComponent::m_connectToHostFailedUi, "Connection To Host Failed", "Ui to inform the user that connecting to the host failed.")
                    ;
            }
        }
    }

    void UiStartMenuComponent::Activate()
    {
        UiCursorBus::Broadcast(&UiCursorInterface::IncrementVisibleCounter);

        // Listen for button presses
        UiButtonBus::Event(m_exitButtonUi, &UiButtonInterface::SetOnClickCallback, [this](AZ::EntityId buttonEntityId, [[maybe_unused]] AZ::Vector2 position) {OnButtonClicked(buttonEntityId); });
        UiButtonBus::Event(m_hostButtonUi, &UiButtonInterface::SetOnClickCallback, [this](AZ::EntityId buttonEntityId, [[maybe_unused]] AZ::Vector2 position) {OnButtonClicked(buttonEntityId); });
        UiButtonBus::Event(m_joinButtonUi, &UiButtonInterface::SetOnClickCallback, [this](AZ::EntityId buttonEntityId, [[maybe_unused]] AZ::Vector2 position) {OnButtonClicked(buttonEntityId); });
        UiButtonBus::Event(m_connectToHostFailedUi, &UiButtonInterface::SetOnClickCallback, [this](AZ::EntityId buttonEntityId, [[maybe_unused]] AZ::Vector2 position) {OnButtonClicked(buttonEntityId); });

        // Hide the host button if this app can't host
        #if !AZ_TRAIT_SERVER
            UiElementBus::Event(m_hostButtonUi, &UiElementInterface::SetIsEnabled, false);
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
                AZ_Warning("UiStartMenuComponent", false, "Could not access cl_serveraddr cvar, so the ip-address input textfield won't be filled out. Please update UiStartMenuComponent.cpp to check for a valid cvar!")
            }
        }
    }

    void UiStartMenuComponent::Deactivate()
    {
        m_onConnectToHostFailed.Disconnect();
        UiCursorBus::Broadcast(&UiCursorInterface::DecrementVisibleCounter);
    }

    void UiStartMenuComponent::OnButtonClicked(AZ::EntityId buttonEntityId) const
    {
        const auto console = AZ::Interface<AZ::IConsole>::Get();
        if (!console)
        {
            AZ_Assert(false, "UiStartMenuComponent attempting to use console commands before AZ::Console is available.");
            return;
        }

        if (buttonEntityId == m_exitButtonUi)
        {
            console->PerformCommand("quit");
        }
        else if (buttonEntityId == m_hostButtonUi)
        {
            console->PerformCommand("host");
            console->PerformCommand(AZStd::string::format("loadlevel %s", MultiplayerLevelName).c_str());
        }
        else if (buttonEntityId == m_joinButtonUi)
        {
            UiElementBus::Event(m_attemptConnectionBlockerUi, &UiElementInterface::SetIsEnabled, true);

            AZStd::string ipAddress = "";
            UiTextInputBus::EventResult(ipAddress, m_ipAddressTextInputUi, &UiTextInputInterface::GetText);

            const AZStd::string connectCommand = AZStd::string::format("connect %s", ipAddress.c_str());
            console->PerformCommand(connectCommand.c_str());
        }
        else if (buttonEntityId == m_connectToHostFailedUi)
        {
            // Player acknowledged connection failed. Close the warning popup.
            UiElementBus::Event(m_connectToHostFailedUi, &UiElementInterface::SetIsEnabled, false);
        }
    }

    void UiStartMenuComponent::OnConnectToHostFailed()
    {
        UiElementBus::Event(m_attemptConnectionBlockerUi, &UiElementInterface::SetIsEnabled, false);
        UiElementBus::Event(m_connectToHostFailedUi, &UiElementInterface::SetIsEnabled, true);
    }

} // namespace MultiplayerSample
