/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <UiPlayerArmorBus.h>
#include <AzCore/Serialization/EditContext.h>

#include <LyShine/Bus/UiImageBus.h>
#include <LyShine/Bus/UiTextBus.h>

#include <Source/Components/UI/UiPlayerArmorComponent.h>

namespace MultiplayerSample
{
    class BehaviorUiPlayerArmorNotificationBusHandler
        : public UiPlayerArmorNotificationBus::Handler
        , public AZ::BehaviorEBusHandler
    {
    public:
        AZ_EBUS_BEHAVIOR_BINDER(BehaviorUiPlayerArmorNotificationBusHandler, "{6EB46F80-3D88-4DB9-8446-348A2CB4320D}", AZ::SystemAllocator,
            OnPlayerArmorChanged);

        void OnPlayerArmorChanged(float armorPointsForLocalPlayer, float startingArmor) override
        {
            Call(FN_OnPlayerArmorChanged, armorPointsForLocalPlayer, startingArmor);
        }
    };


    void UiPlayerArmorComponent::Activate()
    {
        UiPlayerArmorNotificationBus::Handler::BusConnect();
        PlayerIdentityNotificationBus::Handler::BusConnect();

        // Attempt to grab the player name now, but it's likely the UI is loaded before a player name is chosen
        const char* playerName = nullptr;
        PlayerIdentityRequestBus::BroadcastResult(playerName, &PlayerIdentityRequestBus::Events::GetPlayerIdentityName);
        if (playerName != nullptr)
        {
            UiTextBus::Event(m_playerName, &UiTextBus::Events::SetText, playerName);
        }
    }

    void UiPlayerArmorComponent::Deactivate()
    {
        PlayerIdentityNotificationBus::Handler::BusDisconnect();
        UiPlayerArmorNotificationBus::Handler::BusDisconnect();
    }

    void UiPlayerArmorComponent::OnAutonomousPlayerNameChanged(const char* playerName)
    {
        UiTextBus::Event(m_playerName, &UiTextBus::Events::SetText, playerName);
    }

    void UiPlayerArmorComponent::OnPlayerArmorChanged(float armorPointsForLocalPlayer, float startingArmor)
    {
        const AZStd::string armorTextValue = AZStd::string::format("%.0f", armorPointsForLocalPlayer) + AZStd::string("%");
        UiTextBus::Event(m_armorText, &UiTextBus::Events::SetText, armorTextValue.c_str());

        if (startingArmor > 1.f)
        {
            const float fillAmount = AZStd::clamp(armorPointsForLocalPlayer / startingArmor, 0.f, 1.f);
            UiImageBus::Event(m_armorVisualEntity, &UiImageBus::Events::SetFillAmount, fillAmount);
        }
        else
        {
            UiImageBus::Event(m_armorVisualEntity, &UiImageBus::Events::SetFillAmount, 1.f);
        }
    }

    void UiPlayerArmorComponent::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<UiPlayerArmorComponent, AZ::Component>()
                ->Version(1)
                ->Field("Root Element", &UiPlayerArmorComponent::m_rootElement)
                ->Field("Player Armor Visual", &UiPlayerArmorComponent::m_armorVisualEntity)
                ->Field("Player Armor Text", &UiPlayerArmorComponent::m_armorText)
                ->Field("Player Name Text", &UiPlayerArmorComponent::m_playerName)
                ;

            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                editContext->Class<UiPlayerArmorComponent>("UiPlayerArmorComponent",
                    "Shows how much armor the local player has.")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC_CE("UI"))

                    ->DataElement(AZ::Edit::UIHandlers::Default, &UiPlayerArmorComponent::m_rootElement,
                        "Root Element",
                        "Top level element that contains all other elements for player's armor value")
                    ->DataElement(nullptr, &UiPlayerArmorComponent::m_armorVisualEntity, "Player Armor Visual", "")
                    ->DataElement(nullptr, &UiPlayerArmorComponent::m_armorText, "Player Armor Text", "")
                    ->DataElement(nullptr, &UiPlayerArmorComponent::m_playerName, "Player Name Text", "")

                    ;
            }
        }

        AZ::BehaviorContext* behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context);
        if (behaviorContext)
        {
            behaviorContext->EBus<UiPlayerArmorNotificationBus>("UiPlayerArmorNotificationBus")
                ->Handler<BehaviorUiPlayerArmorNotificationBusHandler>();
        }
    }
} // namespace MultiplayerSample
