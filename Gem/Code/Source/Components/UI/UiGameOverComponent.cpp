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
                ->Field("Rank Numbers Elements", &UiGameOverComponent::m_rankNumbersUIContainer)
                ->Field("Top Player Elements", &UiGameOverComponent::m_topPlayersUIElements)
                ->Field("Time Remaining Until New Match UI Elements", &UiGameOverComponent::m_timeRemainingUntilNewMatchUIContainer)
                ;

            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                editContext->Class<UiGameOverComponent>("Ui Game Over", "Shows Game Over screen and results")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC_CE("CanvasUI"))
                    ->DataElement(AZ::Edit::UIHandlers::Default, &UiGameOverComponent::m_gameOverRootElement,
                        "Game Over root element", "The root element of all Game Over specific UI.")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &UiGameOverComponent::m_rankNumbersUIContainer,
                        "Rank Numbers", "Used to display the rank of this client's autonomous player. The parent container UI element containing ui images for each possible rank. Example 1st-10th place.")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &UiGameOverComponent::m_topPlayersUIElements,
                        "Top Players", "A sorted list of rows to display the top players (player rank 1-3). Each row should have 2 children: name and score.")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &UiGameOverComponent::m_timeRemainingUntilNewMatchUIContainer,
                        "Time Remaining Until New Match UI Elements", "A container sorted list number images to display the remaining time until the new match starts.")
                    ;
            }
        }
    }

    void UiGameOverComponent::Activate()
    {
        m_waitForActiveNetworkMatchComponent.Enqueue(AZ::TimeMs{ 1000 }, true);
        UiGameOverBus::Handler::BusConnect(GetEntityId());
    }

    void UiGameOverComponent::Deactivate()
    {
        UiGameOverBus::Handler::BusDisconnect();
        m_waitForActiveNetworkMatchComponent.RemoveFromQueue();
        m_onTimeRemainingChanged.RemoveFromQueue();
        m_onRoundNumberChangedHandler.Disconnect();
    }

    void UiGameOverComponent::DisplayTimeRemainingUI(uint16_t secondsRemaining)
    {
        LyShine::EntityArray rankNumberUIElements;
        UiElementBus::EventResult(rankNumberUIElements, m_timeRemainingUntilNewMatchUIContainer, &UiElementBus::Events::GetChildElements);

        for(uint16_t uiNumbersIdx = 0; uiNumbersIdx < rankNumberUIElements.size(); ++uiNumbersIdx)
        {
            UiElementBus::Event(rankNumberUIElements[uiNumbersIdx]->GetId(), &UiElementBus::Events::SetIsEnabled, secondsRemaining == uiNumbersIdx);
        }
    }

    void UiGameOverComponent::SetGameOverScreenEnabled(bool enabled)
    {
        if (enabled)
        {
            m_timeRemainingUntilNewMatch = RestTimeBetweenMatches;
            DisplayTimeRemainingUI(m_timeRemainingUntilNewMatch);
            m_onTimeRemainingChanged.Enqueue(AZ::TimeMs{ 1000 }, true);
        }
        else
        {
            m_onTimeRemainingChanged.RemoveFromQueue();
        }

        UiElementBus::Event(m_gameOverRootElement, &UiElementBus::Events::SetIsEnabled, enabled);
    }

    void UiGameOverComponent::DisplayResults([[maybe_unused]]MatchResultsSummary results)
    {
        //UiTextBus::Event(m_winnerNameElement, &UiTextBus::Events::SetText, results.m_winningPlayerName.c_str());
        //auto resultsSummary = BuildResultsSummary(results.m_playerStates);
        //UiTextBus::Event(m_matchResultsElement, &UiTextBus::Events::SetText, resultsSummary);
    }
}
