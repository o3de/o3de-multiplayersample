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

#if AZ_TRAIT_CLIENT
#include <LyShine/Bus/UiCanvasBus.h>
#include <LyShine/Bus/UiTextBus.h>
#endif

namespace MultiplayerSample
{
    void HUDComponent::Activate()
    {
#if AZ_TRAIT_CLIENT
        UiCanvasRefBus::EventResult(m_uiCanvasId, GetEntityId(), &UiCanvasRefBus::Events::GetCanvas);
        if (!m_uiCanvasId.IsValid())
        {
            UiCanvasAssetRefNotificationBus::Handler::BusConnect(GetEntityId());
        }

        if (NetworkMatchComponent* netMatchComponent = GetEntity()->FindComponent<NetworkMatchComponent>())
        {
            SetRoundNumberText(netMatchComponent->GetRoundNumber());
            m_roundNumberHandler = AZ::EventHandler<uint16_t>([this](uint16_t value) { SetRoundNumberText(value); });
            netMatchComponent->RoundNumberAddEvent(m_roundNumberHandler);

            m_roundTimerHandler = AZ::EventHandler<RoundTimeSec>([this](RoundTimeSec value) { SetRoundTimerText(value); });
            netMatchComponent->RoundTimeAddEvent(m_roundTimerHandler);
        }
#endif
    }

    void HUDComponent::Deactivate()
    {
#if AZ_TRAIT_CLIENT
        UiCanvasAssetRefNotificationBus::Handler::BusDisconnect(GetEntityId());
#endif
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
        if (m_uiCanvasId.IsValid())
        {
            AZ::Entity* textBoxEntity = nullptr;
            UiCanvasBus::EventResult(textBoxEntity, m_uiCanvasId, &UiCanvasBus::Events::FindElementById, m_roundNumberId);

            if (textBoxEntity != nullptr)
            {
                if (const NetworkMatchComponent* netMatchComponent = GetEntity()->FindComponent<NetworkMatchComponent>())
                {
                    m_roundNumberText = AZStd::string::format("%d of %d", round, netMatchComponent->GetTotalRounds());
                    UiTextBus::Event(textBoxEntity->GetId(), &UiTextBus::Events::SetText, m_roundNumberText);
                }
            }
        }
    }

    void HUDComponent::SetRoundTimerText(RoundTimeSec time)
    {
        if (m_uiCanvasId.IsValid())
        {
            AZ::Entity* textBoxEntity;
            UiCanvasBus::EventResult(textBoxEntity, m_uiCanvasId, &UiCanvasBus::Events::FindElementById, m_roundTimerId);

            if (textBoxEntity != nullptr)
            {
                auto duration = AZStd::chrono::seconds(time);
                auto minutes = AZStd::chrono::duration_cast<AZStd::chrono::minutes>(duration);
                auto seconds = AZStd::chrono::duration_cast<AZStd::chrono::seconds>(duration - minutes);

                m_roundTimerText = AZStd::string::format("%02i:%02i", static_cast<int>(minutes.count()), static_cast<int>(seconds.count()));

                UiTextBus::Event(textBoxEntity->GetId(), &UiTextBus::Events::SetText, m_roundTimerText);
            }
        }
    }
#endif
} // namespace MultiplayerSample
