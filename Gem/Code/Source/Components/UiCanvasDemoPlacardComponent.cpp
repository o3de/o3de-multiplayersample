/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <Source/Components/UiCanvasDemoPlacardComponent.h>

#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>

#include <LyShine/Bus/UiCanvasBus.h>
#include <LyShine/Bus/UiTextBus.h>

namespace MultiplayerSample
{
    void UiCanvasDemoPlacardComponent::Activate()
    {
        AZ::EntityId uiCanvasEntityId;
        UiCanvasRefBus::EventResult(uiCanvasEntityId, GetEntityId(), &UiCanvasRefBus::Events::GetCanvas);
        if (uiCanvasEntityId.IsValid())
        {
            SetUiCanvasText(uiCanvasEntityId);
        }
        else
        {
            UiCanvasAssetRefNotificationBus::Handler::BusConnect(GetEntityId());
        }
    }

    void UiCanvasDemoPlacardComponent::Deactivate()
    {
        UiCanvasAssetRefNotificationBus::Handler::BusDisconnect(GetEntityId());
    }

    void UiCanvasDemoPlacardComponent::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<UiCanvasDemoPlacardComponent, AZ::Component>()
                ->Version(1)
                ->Field("Text", &UiCanvasDemoPlacardComponent::m_placardText)
                ->Field("TextboxUiId", &UiCanvasDemoPlacardComponent::m_placardTextboxUiElementId)
                ;

            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                editContext->Class<UiCanvasDemoPlacardComponent>("UiCanvasDemoPlacardComponent", "Helper component for setting up UI canvases placards on meshes in and around the world. This component can dynamically change the placard text so that we can reuse one UiCanvas for multiple placards.")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ->Attribute(AZ::Edit::Attributes::Category, "ComponentCategory")
                        ->Attribute(AZ::Edit::Attributes::Icon, "Icons/Components/Component_Placeholder.svg")
                        ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC_CE("Game"))

                    ->DataElement(AZ::Edit::UIHandlers::Default, &UiCanvasDemoPlacardComponent::m_placardTextboxUiElementId, "Textbox UIElement Id", "The element id of the textbox you want to change. (Find the id inside the ui canvas)")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &UiCanvasDemoPlacardComponent::m_placardText, "Placard Text", "Setting this text will change the textbox's text.")
                    ;
            }
        }
    }
    
    void UiCanvasDemoPlacardComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        required.push_back(AZ_CRC("UiCanvasRefService"));
    }

    void UiCanvasDemoPlacardComponent::OnCanvasLoadedIntoEntity(AZ::EntityId uiCanvasEntity)
    {
        SetUiCanvasText(uiCanvasEntity);
    }

    void UiCanvasDemoPlacardComponent::SetUiCanvasText(AZ::EntityId uiCanvasEntityId)
    {
        AZ::Entity* textBoxEntity;
        UiCanvasBus::EventResult(textBoxEntity, uiCanvasEntityId, &UiCanvasBus::Events::FindElementById, m_placardTextboxUiElementId);

        if (textBoxEntity != nullptr)
        {
            UiTextBus::Event(textBoxEntity->GetId(), &UiTextBus::Events::SetText, m_placardText);
        }
    }
} // namespace MultiplayerSample
