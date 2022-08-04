/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AzCore/Serialization/EditContext.h>
#include <LyShine/Bus/UiTextBus.h>
#include <Source/Components/UI/UiCoinCountComponent.h>

namespace MultiplayerSample
{
    void UiCoinCountComponent::Activate()
    {
        UiCoinCountNotificationBus::Handler::BusConnect();
    }

    void UiCoinCountComponent::Deactivate()
    {
        UiCoinCountNotificationBus::Handler::BusDisconnect();
    }

    void UiCoinCountComponent::OnCoinCountChanged(uint16_t totalCoinsCollectedByLocalPlayer)
    {
        const AZStd::string message = AZStd::string::format("%d", totalCoinsCollectedByLocalPlayer);
        UiTextBus::Event(m_coinsTextForLocalPlayer, &UiTextBus::Events::SetText, message);
    }

    void UiCoinCountComponent::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<UiCoinCountComponent, AZ::Component>()
                ->Version(1)
                ->Field("Coins Text For Local Player", &UiCoinCountComponent::m_coinsTextForLocalPlayer)
                ;

            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                editContext->Class<UiCoinCountComponent>("Ui Coin Count", 
                    "Updates collected coin count for the local player")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC_CE("CanvasUI"))

                    ->DataElement(AZ::Edit::UIHandlers::Default, &UiCoinCountComponent::m_coinsTextForLocalPlayer, 
                        "Coins Text For Local Player", 
                        "The text element for the count of coins collected by the local player.")
                    ;
            }
        }
    }
} // namespace MultiplayerSample
