/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <Source/Components/UI/HUDComponent.h>

#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>

#include <LyShine/Bus/UiElementBus.h>
#include <LyShine/Bus/UiTextBus.h>


namespace MultiplayerSample
{
    void HUDComponent::Activate()
    {
        m_waitForActiveNetworkMatchComponent.Enqueue(AZ::TimeMs{ 1000 }, true);
    }

    void HUDComponent::Deactivate()
    {
        m_waitForActiveNetworkMatchComponent.RemoveFromQueue();
        m_roundNumberHandler.Disconnect();
        m_roundTimerHandler.Disconnect();
    }

    void HUDComponent::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<HUDComponent, AZ::Component>()
                ->Version(1)
                ->Field("RoundNumberText", &HUDComponent::m_roundNumberText)
                ->Field("RoundNumberId", &HUDComponent::m_roundNumberUi)
                ->Field("RoundTimerId", &HUDComponent::m_roundTimerUi)
                ->Field("RoundSecondsRemaining", &HUDComponent::m_roundSecondsRemainingUiParent)
                ->Field("FirstMatchParent", &HUDComponent::m_firstMatchStartingUiParent)
                ->Field("FirstMatchTimer", &HUDComponent::m_firstMatchStartingTimerUi);

            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                editContext->Class<HUDComponent>("HUDComponent", "Helper component for setting up Multiplayer Sample's HUD")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Category, "Multiplayer Sample UI")
                    ->Attribute(AZ::Edit::Attributes::Icon, "Icons/Components/Component_Placeholder.svg")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC_CE("CanvasUI"))

                    ->DataElement(AZ::Edit::UIHandlers::Default, &HUDComponent::m_roundNumberUi, "Round Number Textbox", "The ui textbox for displaying the current round number.")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &HUDComponent::m_roundTimerUi, "Round Time Textbox", "The ui textbox for displaying the time remaining in the round.")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &HUDComponent::m_roundSecondsRemainingUiParent, "Round Seconds Remaining UI Elements", "The parent ui element containing all the ui images to display the seconds remaining.")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &HUDComponent::m_firstMatchStartingUiParent, "First Match Parent", "UI to display for players while they wait for other players to join before the first match begins.")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &HUDComponent::m_firstMatchStartingTimerUi, "First Match Timer", "The ui text to display the time remaining before the first match starts.")
                    ;
            }
        }
    }

    void HUDComponent::SetRoundNumberText(uint16_t round)
    {
        if (const INetworkMatch* netMatchComponent = AZ::Interface<INetworkMatch>::Get())
        {
            // Display the current round number.
            // The end of match can push the round count over the max round count, so cap it.
            const uint16_t totalRounds = aznumeric_cast<uint16_t>(netMatchComponent->GetTotalRoundCount());
            m_roundNumberText = AZStd::string::format("%d of %d", AZStd::min(round, totalRounds), totalRounds);
            UiTextBus::Event(m_roundNumberUi, &UiTextBus::Events::SetText, m_roundNumberText);
        }
    }

    void HUDComponent::SetRoundTimerText(RoundTimeSec time)
    {
        // Display a clock with the time remaining
        auto duration = AZStd::chrono::seconds(time);
        auto minutes = AZStd::chrono::duration_cast<AZStd::chrono::minutes>(duration);
        auto seconds = AZStd::chrono::duration_cast<AZStd::chrono::seconds>(duration - minutes);

        AZStd::string roundTimerText = AZStd::string::format("%02i:%02i", static_cast<int>(minutes.count()), static_cast<int>(seconds.count()));
        UiTextBus::Event(m_roundTimerUi, &UiTextBus::Events::SetText, roundTimerText);

        // Display a countdown of custom UI when the round is close to finishing
        if (duration.count() > 0 && duration.count() <= 10)
        {
            UiElementBus::Event(m_roundSecondsRemainingUiParent, &UiElementBus::Events::SetIsEnabled, true);

            AZStd::vector<AZ::EntityId> uiSecondsRemainingUIElements;
            UiElementBus::EventResult(uiSecondsRemainingUIElements, m_roundSecondsRemainingUiParent, &UiElementBus::Events::GetChildEntityIds);
            for (int i = 0; i < uiSecondsRemainingUIElements.size(); ++i)
            {
                UiElementBus::Event(uiSecondsRemainingUIElements[i], &UiElementBus::Events::SetIsEnabled, i == aznumeric_cast<int>(duration.count()));
            }
        }
        else
        {
            UiElementBus::Event(m_roundSecondsRemainingUiParent, &UiElementBus::Events::SetIsEnabled, false);
        }
    }

    void HUDComponent::UpdateFirstMatchTimerUi()
    {
        const AZ::TimeMs timeRemainingUntilMatchStartMs = AZ::Interface<INetworkMatch>::Get()->GetMatchStartHostTime() - AZ::Interface<Multiplayer::IMultiplayer>::Get()->GetCurrentHostTimeMs();

        // Update the UI to display the time remaining until the first match begins
        if (timeRemainingUntilMatchStartMs > AZ::Time::ZeroTimeMs)
        {
            UiElementBus::Event(m_firstMatchStartingUiParent, &UiElementBus::Events::SetIsEnabled, true);

            const AZStd::chrono::milliseconds duration(static_cast<long long>(timeRemainingUntilMatchStartMs));
            const auto minutes = AZStd::chrono::duration_cast<AZStd::chrono::minutes>(duration);
            const auto seconds = AZStd::chrono::duration_cast<AZStd::chrono::seconds>(duration - minutes);

            AZStd::string matchTimeText = AZStd::string::format("%02i:%02i", static_cast<int>(minutes.count()), static_cast<int>(seconds.count()));
            UiTextBus::Event(m_firstMatchStartingTimerUi, &UiTextBus::Events::SetText, matchTimeText);

            // Requeue to refresh the UI after a second if there's still time left on the clock
            if (!m_updateFirstMatchTimer.IsScheduled())
            {
                m_updateFirstMatchTimer.Enqueue(AZ::SecondsToTimeMs(1.0), true);
            }
        }
        else
        {
            UiElementBus::Event(m_firstMatchStartingUiParent, &UiElementBus::Events::SetIsEnabled, false);
            m_updateFirstMatchTimer.RemoveFromQueue();
        }
    }
} // namespace MultiplayerSample
