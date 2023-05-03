/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <MultiplayerSampleTypes.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Settings/SettingsRegistry.h>
#if AZ_TRAIT_CLIENT
#include <LyShine/Bus/UiTextBus.h>
#endif
#include <Source/Components/UI/UiCoinCountComponent.h>

namespace MultiplayerSample
{
    void UiCoinCountComponent::Activate()
    {
#if AZ_TRAIT_CLIENT
        UiCoinCountNotificationBus::Handler::BusConnect();
#endif
    }

    void UiCoinCountComponent::Deactivate()
    {
#if AZ_TRAIT_CLIENT
        UiCoinCountNotificationBus::Handler::BusDisconnect();
#endif
    }

#if AZ_TRAIT_CLIENT
    void UiCoinCountComponent::OnCoinCountChanged(uint16_t totalCoinsCollectedByLocalPlayer)
    {
        AZ::u64 winningCoinCount = 10;
        if (const auto registry = AZ::SettingsRegistry::Get())
        {
            registry->Get(winningCoinCount, WinningCoinCountSetting);
        }

        const AZStd::string message = AZStd::string::format("%04d", totalCoinsCollectedByLocalPlayer);
        UiTextBus::Event(m_coinsTextForLocalPlayer, &UiTextBus::Events::SetText, message);

        if (m_coinTextColorEffectDuration > AZ::Time::ZeroTimeMs) // sanity check
        {
            m_coinTextColorEffect = m_coinTextColorEffectDuration;
            m_gameFrameTick.Enqueue(AZ::Time::ZeroTimeMs, true);
        }
        else
        {
            UiTextBus::Event(m_coinsTextForLocalPlayer, &UiTextBus::Events::SetColor, m_coinTextColor);
        }
    }

    void UiCoinCountComponent::OnTick(AZ::TimeMs delta)
    {
        m_coinTextColorEffect -= delta;
        if (m_coinTextColorEffect <= AZ::Time::ZeroTimeMs)
        {
            UiTextBus::Event(m_coinsTextForLocalPlayer, &UiTextBus::Events::SetColor, m_coinTextColor);
            m_gameFrameTick.RemoveFromQueue();
        }
        else
        {
            const float normalizedEffectTime = aznumeric_cast<float>(m_coinTextColorEffect) / aznumeric_cast<float>(m_coinTextColorEffectDuration);
            const AZ::Color interpolatedColor = m_coinTextColor.Lerp(m_recentlyChangedCoinTextColor, normalizedEffectTime);
            UiTextBus::Event(m_coinsTextForLocalPlayer, &UiTextBus::Events::SetColor, interpolatedColor);
        }
    }
#endif

    void UiCoinCountComponent::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<UiCoinCountComponent, AZ::Component>()
                ->Version(1)
                ->Field("Coin Count Text", &UiCoinCountComponent::m_coinsTextForLocalPlayer)
                ->Field("Just Changed Color", &UiCoinCountComponent::m_recentlyChangedCoinTextColor)
                ->Field("Coin Color", &UiCoinCountComponent::m_coinTextColor)
                ->Field("Effect Duration", &UiCoinCountComponent::m_coinTextColorEffectDuration)
                ;

            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                editContext->Class<UiCoinCountComponent>("Ui Coin Count",
                    "Updates collected coin count for the local player")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC_CE("CanvasUI"))

                    ->DataElement(AZ::Edit::UIHandlers::Default, &UiCoinCountComponent::m_coinsTextForLocalPlayer,
                        "Coin Count Text",
                        "The text element for the count of coins collected by the local player.")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &UiCoinCountComponent::m_recentlyChangedCoinTextColor,
                        "Just Changed Color",
                        "Briefly show the coin count in this color whenever a coin count changes.")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &UiCoinCountComponent::m_coinTextColor,
                        "Coin Color",
                        "The common text color to use for coin count.")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &UiCoinCountComponent::m_coinTextColorEffectDuration,
                        "Effect Duration",
                        "The duration of the color change effect when a coin count changes.")
                    ->Attribute(AZ::Edit::Attributes::Suffix, " milliseconds")
                    ;
            }
        }
    }
} // namespace MultiplayerSample
