/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <Source/Components/UI/HUDComponent.h>
#include <Source/Components/NetworkMatchComponent.h>

#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>

#include <LyShine/Bus/UiTextBus.h>

namespace MultiplayerSample
{
    void HUDComponent::Activate()
    {
            netMatchComponent->RoundTimeAddEvent(m_roundTimerHandler);
        }
#endif
    }

    void HUDComponent::Deactivate()
    {
        m_waitForActiveNetworkMatchComponent.RemoveFromQueue();
    }

    void HUDComponent::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<HUDComponent, AZ::Component>()
                ->Version(1)
                ->Field("RoundNumberText", &HUDComponent::m_roundNumberText)
                ->Field("RoundNumberId", &HUDComponent::m_roundNumberUi)
                ->Field("RoundTimerText", &HUDComponent::m_roundTimerText)
                ->Field("RoundTimerId", &HUDComponent::m_roundTimerUi)
                ->Field("RoundSecondsRemaining", &HUDComponent::m_roundSecondsRemainingUiParent)
                ;

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
                    ;
            }
        }
    }
    
    void HUDComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        required.push_back(AZ_CRC("UiCanvasRefService"));
        required.push_back(AZ_CRC("NetworkMatchComponent"));
    }

#if AZ_TRAIT_CLIENT
    void HUDComponent::OnCanvasLoadedIntoEntity(AZ::EntityId uiCanvasEntity)
    {
        m_uiCanvasId = uiCanvasEntity;
    }

    void HUDComponent::SetRoundNumberText(uint16_t round)
    {
        if (const NetworkMatchComponent* netMatchComponent = AZ::Interface<NetworkMatchComponent>::Get())
        {
            // Display the current round number.
            // The end of match can push the round count over the max round count, so cap it.
            const uint16_t totalRounds = netMatchComponent->GetTotalRounds();
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

        m_roundTimerText = AZStd::string::format("%02i:%02i", static_cast<int>(minutes.count()), static_cast<int>(seconds.count()));

        UiTextBus::Event(m_roundTimerUi, &UiTextBus::Events::SetText, m_roundTimerText);

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
} // namespace MultiplayerSample
