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

#include <LyShine/Bus/UiCanvasBus.h>
#include <LyShine/Bus/UiTextBus.h>

namespace MultiplayerSample
{
    void HUDComponent::Activate()
    {
        UiCanvasRefBus::EventResult(m_uiCanvasId, GetEntityId(), &UiCanvasRefBus::Events::GetCanvas);
        if (!m_uiCanvasId.IsValid())
        {
            UiCanvasAssetRefNotificationBus::Handler::BusConnect(GetEntityId());
        }

        NetworkMatchComponent* netMatchComponent = GetEntity()->FindComponent<NetworkMatchComponent>();

        SetRoundNumberText(netMatchComponent->GetRoundNumber());
        m_roundNumberHandler = AZ::EventHandler<uint16_t>([this](uint16_t value) { this->SetRoundNumberText(value); });
        netMatchComponent->RoundNumberAddEvent(m_roundNumberHandler);

        m_roundTimerHandler = AZ::EventHandler<float>([this](float value) { this->SetRoundTimerText(value); });
        netMatchComponent->RoundTimeAddEvent(m_roundTimerHandler);
    }

    void HUDComponent::Deactivate()
    {
        UiCanvasAssetRefNotificationBus::Handler::BusDisconnect(GetEntityId());
    }

    void HUDComponent::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<HUDComponent, AZ::Component>()
                ->Version(1)
                ->Field("RoundNumberText", &HUDComponent::m_roundNumberText)
                ->Field("RoundNumberId", &HUDComponent::m_roundNumberId)
                ->Field("RoundTimerText", &HUDComponent::m_roundTimerText)
                ->Field("RoundTimerId", &HUDComponent::m_roundTimerId)
                ;

            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                editContext->Class<HUDComponent>("HUDComponent", "Helper component for setting up Multiplayer Sample's HUD")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Category, "Multiplayer Sample UI")
                    ->Attribute(AZ::Edit::Attributes::Icon, "Icons/Components/Component_Placeholder.svg")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC_CE("Game"))

                    ->DataElement(AZ::Edit::UIHandlers::Default, &HUDComponent::m_roundNumberId, "Round Number UIElement Id", "The element id of the textbox you want to change. (Find the id inside the ui canvas)")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &HUDComponent::m_roundTimerId, "Round Time UIElement Id", "The element id of the textbox you want to change. (Find the id inside the ui canvas)")
                    ;
            }
        }
    }
    
    void HUDComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        required.push_back(AZ_CRC("UiCanvasRefService"));
        required.push_back(AZ_CRC("NetworkMatchComponent"));
    }

    void HUDComponent::OnCanvasLoadedIntoEntity(AZ::EntityId uiCanvasEntity)
    {
        m_uiCanvasId = uiCanvasEntity;
    }

    void HUDComponent::SetRoundNumberText(uint16_t round)
    {
        if (m_uiCanvasId.IsValid())
        {
            AZ::Entity* textBoxEntity;
            UiCanvasBus::EventResult(textBoxEntity, m_uiCanvasId, &UiCanvasBus::Events::FindElementById, m_roundNumberId);

            if (textBoxEntity != nullptr)
            {
                NetworkMatchComponent* netMatchComponent = GetEntity()->FindComponent<NetworkMatchComponent>();
                m_roundNumberText = AZStd::string::format("%d/%d", round, netMatchComponent->GetTotalRounds());
                UiTextBus::Event(textBoxEntity->GetId(), &UiTextBus::Events::SetText, m_roundNumberText);
            }
        }
    }

    void HUDComponent::SetRoundTimerText(float time)
    {
        if (m_uiCanvasId.IsValid())
        {
            AZ::Entity* textBoxEntity;
            UiCanvasBus::EventResult(textBoxEntity, m_uiCanvasId, &UiCanvasBus::Events::FindElementById, m_roundTimerId);

            if (textBoxEntity != nullptr)
            {
                m_roundTimerText = AZStd::string::format("%d", aznumeric_cast<int>(time));
                UiTextBus::Event(textBoxEntity->GetId(), &UiTextBus::Events::SetText, m_roundTimerText);
            }
        }
    }
} // namespace MultiplayerSample
