/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AzCore/Serialization/EditContext.h>
#include <LyShine/Bus/UiElementBus.h>
#include <LyShine/Bus/UiCursorBus.h>
#include <LyShine/Bus/UiTextBus.h>
#include <Source/Components/UI/UiGameOverComponent.h>

namespace MultiplayerSample
{
    void UiGameOverComponent::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<UiGameOverComponent, AZ::Component>()
                ->Version(2)
                ->Field("Game Over root element", &UiGameOverComponent::m_gameOverRootElement)
                ->Field("Winner Name Text", &UiGameOverComponent::m_winnerNameElement)
                ->Field("Match Results element", &UiGameOverComponent::m_matchResultsElement)
                ->Field("Close Results Button", &UiGameOverComponent::m_closeResultsButton)
                ;

            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                editContext->Class<UiGameOverComponent>("Ui Game Over", "Shows Game Over screen and results")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC_CE("CanvasUI"))
                    ->DataElement(AZ::Edit::UIHandlers::Default, &UiGameOverComponent::m_gameOverRootElement,
                        "Game Over root element", "The root element of all Game Over specific UI")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &UiGameOverComponent::m_winnerNameElement,
                        "Winner Name Text", "The text element for the name of the match winner")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &UiGameOverComponent::m_matchResultsElement,
                        "Match Results element", "The element for the textual display of the match results")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &UiGameOverComponent::m_closeResultsButton,
                        "Close Results Button", "The button to close results UI")
                    ;
            }
        }
    }

    void UiGameOverComponent::Activate()
    {
        UiGameOverBus::Handler::BusConnect(GetEntityId());
        UiButtonNotificationBus::Handler::BusConnect(m_closeResultsButton);
        UiCursorBus::Broadcast(&UiCursorBus::Events::SetUiCursor, "UICanvases/cursor.png");
    }

    void UiGameOverComponent::Deactivate()
    {
        UiGameOverBus::Handler::BusDisconnect();
        UiButtonNotificationBus::Handler::BusDisconnect();
    }

    void UiGameOverComponent::SetGameOverScreenEnabled(bool enabled)
    {
        UiElementBus::Event(m_gameOverRootElement, &UiElementBus::Events::SetIsEnabled, enabled);

        if (enabled)
        {
            UiCursorBus::Broadcast(&UiCursorBus::Events::IncrementVisibleCounter);
        }
        else
        {
            UiCursorBus::Broadcast(&UiCursorBus::Events::DecrementVisibleCounter);
        }
    }

    void UiGameOverComponent::DisplayResults(MatchResultsSummary results)
    {
        UiTextBus::Event(m_winnerNameElement, &UiTextBus::Events::SetText, results.m_winningPlayerName.c_str());
        auto resultsSummary = BuildResultsSummary(results.m_playerStates);
        UiTextBus::Event(m_matchResultsElement, &UiTextBus::Events::SetText, resultsSummary);
    }

    void UiGameOverComponent::OnButtonClick()
    {
        const AZ::EntityId buttonId = *UiButtonNotificationBus::GetCurrentBusId();
        if (buttonId == m_closeResultsButton)
        {
            SetGameOverScreenEnabled(false);
        }
    }

    AZStd::string UiGameOverComponent::BuildResultsSummary(const AZStd::vector<PlayerState>& playerStates)
    {
        // TODO: make this a nice grid in UiCanvas instead of a big string
        AZStd::string resultTable = "---------- Final Standings ----------\n\n";
        for (PlayerState result : playerStates)
        {
            resultTable.append(
                AZStd::string::format("%s :  score  %i, armor %i\n",
                    result.m_playerName.c_str(), result.m_score, result.m_remainingShield));
        }
        return resultTable;
    }
}