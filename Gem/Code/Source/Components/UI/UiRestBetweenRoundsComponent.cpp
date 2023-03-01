/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AzCore/Serialization/EditContext.h>

#if AZ_TRAIT_CLIENT
#include <LyShine/Bus/UiElementBus.h>
#endif

#include <Source/Components/UI/UiRestBetweenRoundsComponent.h>

namespace MultiplayerSample
{
    void UiRestBetweenRoundsComponent::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<UiRestBetweenRoundsComponent, AZ::Component>()
                ->Version(2)
                ->Field("Rest Time Root Ui Element", &UiRestBetweenRoundsComponent::m_restTimerRootUiElement)
                ->Field("Number Container Ui Element", &UiRestBetweenRoundsComponent::m_numbersContainerUiElement)
                ;

            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                editContext->Class<UiRestBetweenRoundsComponent>("Ui Rest Between Rounds", "Shows the rest time remaining before the new round begins")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC_CE("CanvasUI"))
                    ->DataElement(AZ::Edit::UIHandlers::Default, &UiRestBetweenRoundsComponent::m_restTimerRootUiElement,
                        "Rest Time Root", "The root element containing the rest timer ui.")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &UiRestBetweenRoundsComponent::m_numbersContainerUiElement,
                        "Time remaining UI Elements", "The parent containing all the timer numbers")
                    ;
            }
        }
    }

    void UiRestBetweenRoundsComponent::Activate()
    {
#if AZ_TRAIT_CLIENT
        UiRoundsLifecycleBus::Handler::BusConnect(GetEntityId());
#endif
    }

    void UiRestBetweenRoundsComponent::Deactivate()
    {
#if AZ_TRAIT_CLIENT
        UiRoundsLifecycleBus::Handler::BusDisconnect();
#endif
    }

#if AZ_TRAIT_CLIENT
    void UiRestBetweenRoundsComponent::OnRoundRestTimeRemainingChanged(RoundTimeSec secondsRenaming)
    {
        bool enableRootElement = secondsRenaming > 0;
        UiElementBus::Event(m_restTimerRootUiElement, &UiElementBus::Events::SetIsEnabled, enableRootElement);

        LyShine::EntityArray numbersUiElements;
        UiElementBus::EventResult(numbersUiElements, m_numbersContainerUiElement, &UiElementBus::Events::GetChildElements);

        const size_t uiElementCount = numbersUiElements.size();
        for(size_t i = 0; i < uiElementCount; ++i)
        {
            bool enabled = i == aznumeric_cast<size_t>(secondsRenaming);
            UiElementBus::Event(numbersUiElements[i]->GetId(), &UiElementBus::Events::SetIsEnabled, enabled);
        }
    }
#endif
}