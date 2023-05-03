/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AzCore/Serialization/EditContext.h>
#include <AzCore/std/sort.h>

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
                ->Field("Top Player Elements", &UiGameOverComponent::m_topRankPlayersUIElements)
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
                    ->DataElement(AZ::Edit::UIHandlers::Default, &UiGameOverComponent::m_topRankPlayersUIElements,
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
        m_onSecondsRemainingChanged.RemoveFromQueue();
        m_onRoundNumberChangedHandler.Disconnect();
    }

    void UiGameOverComponent::DisplaySecondsRemainingUI()
    {
        const AZ::TimeMs currentHostTime = AZ::Interface<Multiplayer::IMultiplayer>::Get()->GetCurrentHostTimeMs();
        const AZ::TimeMs matchStartTime = AZ::Interface<INetworkMatch>::Get()->GetMatchStartHostTime();

        const uint16_t secondsRemaining = aznumeric_cast<uint16_t>(AZStd::floor(AZ::TimeMsToSeconds(matchStartTime - currentHostTime)));
        LyShine::EntityArray rankNumberUIElements;
        UiElementBus::EventResult(rankNumberUIElements, m_timeRemainingUntilNewMatchUIContainer, &UiElementBus::Events::GetChildElements);

        const uint16_t rankNumberUiCount = aznumeric_cast<uint16_t>(rankNumberUIElements.size());
        for(uint16_t uiNumbersIdx = 0; uiNumbersIdx < rankNumberUiCount; ++uiNumbersIdx)
        {
            UiElementBus::Event(rankNumberUIElements[uiNumbersIdx]->GetId(), &UiElementBus::Events::SetIsEnabled, secondsRemaining == uiNumbersIdx);
        }

        // Remove this event to update the timer ui once the time reaches 0
        if (secondsRemaining == 0)
        {
            m_onSecondsRemainingChanged.RemoveFromQueue();
        }
    }

    void UiGameOverComponent::SetGameOverScreenEnabled(bool enabled)
    {
        if (enabled)
        {
            DisplaySecondsRemainingUI();
            m_onSecondsRemainingChanged.Enqueue(AZ::TimeMs{ 1000 }, true);
        }
        else
        {
            m_onSecondsRemainingChanged.RemoveFromQueue();
        }

        UiElementBus::Event(m_gameOverRootElement, &UiElementBus::Events::SetIsEnabled, enabled);
    }

    void UiGameOverComponent::DisplayResults(MatchResultsSummary results)
    {
        // Sort the players by score (highest score is 1st)
        // If scores are matching, then sort by remaining armor.
        AZStd::sort(results.m_playerStates.begin(), results.m_playerStates.end(), [](const PlayerState& a, const PlayerState& b)
        {
            if (a.m_score == b.m_score)
            {
                return a.m_remainingArmor > b.m_remainingArmor;
            }
            return a.m_score > b.m_score;
        });

        // Display the top 3 players
        for (uint i = 0; i < m_topRankPlayersUIElements.size(); ++i)
        {
            AZ::EntityId& topRankPlayerRow = m_topRankPlayersUIElements[i];

            LyShine::EntityArray playerStatsUiElements;
            UiElementBus::EventResult(playerStatsUiElements, topRankPlayerRow, &UiElementBus::Events::GetChildElements);

            if (playerStatsUiElements.size() >= 2)
            {
                if (i < results.m_playerStates.size())
                {
                    UiTextBus::Event(playerStatsUiElements[0]->GetId(), &UiTextBus::Events::SetText, results.m_playerStates[i].m_playerName.c_str());
                    UiTextBus::Event(playerStatsUiElements[1]->GetId(), &UiTextBus::Events::SetText, AZStd::string::format("%u", results.m_playerStates[i].m_score));

                }
                else
                {
                    UiTextBus::Event(playerStatsUiElements[0]->GetId(), &UiTextBus::Events::SetText, "N/A");
                    UiTextBus::Event(playerStatsUiElements[1]->GetId(), &UiTextBus::Events::SetText, "N/A");
                }
            }
            else
            {
                AZ_Warning("UiGameOverComponent", false, "Failed to display player rank %i. Please update ui canvas so the top player rows have at least 2 children: name and score.", i + 1)
            }
        }

        // Display the rank on this client's player
        const char* playerIdentityName = nullptr;
        PlayerIdentityRequestBus::BroadcastResult(playerIdentityName, &PlayerIdentityRequestBus::Events::GetPlayerIdentityName);

        if (playerIdentityName)
        {
            // Find this client player's rank by matching their name
            uint clientPlayerRank = 0;
            for (; clientPlayerRank < results.m_playerStates.size(); ++clientPlayerRank)
            {
                if (results.m_playerStates[clientPlayerRank].m_playerName == playerIdentityName)
                {
                    break;
                }
            }

            // Display the proper rank number UI
            LyShine::EntityArray rankNumbers;
            UiElementBus::EventResult(rankNumbers, m_rankNumbersUIContainer, &UiElementBus::Events::GetChildElements);
            for (uint i = 0; i < rankNumbers.size(); ++i)
            {
                UiElementBus::Event(rankNumbers[i]->GetId(), &UiElementBus::Events::SetIsEnabled, i == clientPlayerRank);
            }
        }
    }
}
