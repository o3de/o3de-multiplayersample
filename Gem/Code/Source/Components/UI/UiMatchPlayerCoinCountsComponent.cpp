/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AzCore/Serialization/EditContext.h>
#include <AzCore/std/sort.h>

#include <Source/Components/Multiplayer/MatchPlayerCoinsComponent.h>
#include <Components/Multiplayer/PlayerIdentityComponent.h>

#include <LyShine/Bus/UiElementBus.h>
#include <LyShine/Bus/UiTextBus.h>
#include <Multiplayer/Components/NetBindComponent.h>

#include <Source/Components/UI/UiMatchPlayerCoinCountsComponent.h>

namespace MultiplayerSample
{
#if AZ_TRAIT_CLIENT
    const StartingPointInput::InputEventNotificationId ShowPlayerCoinCountsEventId("show_player_coin_counts");
#endif

    void UiMatchPlayerCoinCountsComponent::Activate()
    {
        StartingPointInput::InputEventNotificationBus::MultiHandler::BusConnect(ShowPlayerCoinCountsEventId);
    }

    void UiMatchPlayerCoinCountsComponent::Deactivate()
    {
        StartingPointInput::InputEventNotificationBus::MultiHandler::BusDisconnect();
    }

    void UiMatchPlayerCoinCountsComponent::UpdatePlayerScoreUI()
    {
        // Display player scores sorted by coin count (highest score on top)
        AZStd::vector<PlayerCoinState> coins = AZ::Interface<MatchPlayerCoinsComponent>::Get()->GetPlayerCoinCounts();
        AZStd::sort(coins.begin(), coins.end(), [](const PlayerCoinState& a, const PlayerCoinState& b) {return a.m_coins > b.m_coins; });

        AZStd::size_t elementIndex = 0;
        for (const PlayerCoinState& state : coins)
        {
            if (elementIndex >= m_playerRowElement.size())
            {
                AZ_Error("UiMatchPlayerCoinCounts", false, "Failed to update score screen. Please update UICanvas so there are enough player rows.")
                break;
            }

            AZStd::vector<AZ::EntityId> children;
            UiElementBus::EventResult(children, m_playerRowElement[elementIndex], &UiElementBus::Events::GetChildEntityIds);

            if (children.size() < 3)
            {
                AZ_Error("UiMatchPlayerCoinCounts", false, "Failed to update scorescreen. Please update UICanvas so the player row has at least 3 child elements for setting the player name, coin count, and player highlight.")
                break;
            }

            if (state.m_playerId == Multiplayer::InvalidNetEntityId)
            {
                continue;
            }

            const PlayerNameString name = GetPlayerName(state.m_playerId);
            UiTextBus::Event(children[0], &UiTextBus::Events::SetText, name.c_str());
            UiTextBus::Event(children[1], &UiTextBus::Events::SetText, AZStd::string::format("%d", state.m_coins));

            // Highlight the row belonging to this client's autonomous player
            bool isAutonmousPlayer = false;
            const Multiplayer::ConstNetworkEntityHandle playerHandle = Multiplayer::GetNetworkEntityManager()->GetEntity(state.m_playerId);
            if (playerHandle.Exists() && playerHandle.GetNetBindComponent() != nullptr)
            {
                isAutonmousPlayer = playerHandle.GetNetBindComponent()->IsNetEntityRoleAutonomous();
            }
            UiElementBus::Event(children[2], &UiElementBus::Events::SetIsEnabled, isAutonmousPlayer);

            elementIndex++;
        }

        // Clear out the unused fields.
        for (; elementIndex < m_playerRowElement.size(); ++elementIndex)
        {
            AZStd::vector<AZ::EntityId> children;
            UiElementBus::EventResult(children, m_playerRowElement[elementIndex], &UiElementBus::Events::GetChildEntityIds);
            if (children.size() >= 3)
            {
                UiTextBus::Event(children[0], &UiTextBus::Events::SetText, ""); // name
                UiTextBus::Event(children[1], &UiTextBus::Events::SetText, ""); // score
                UiElementBus::Event(children[2], &UiElementBus::Events::SetIsEnabled, false); // autonomous player highlight
            }
        }
    }

    void UiMatchPlayerCoinCountsComponent::OnPressed([[maybe_unused]] float value)
    {
        const StartingPointInput::InputEventNotificationId* inputId = StartingPointInput::InputEventNotificationBus::GetCurrentBusId();

        if (inputId == nullptr)
        {
            return;
        }

        if (*inputId == ShowPlayerCoinCountsEventId)
        {
            UiElementBus::Event(m_rootElementId, &UiElementBus::Events::SetIsEnabled, true);
            AZ::Interface<MatchPlayerCoinsComponent>::Get()->CoinsPerPlayerAddEvent(m_onPlayerScoreChanged);
            UpdatePlayerScoreUI();
        }
    }

    void UiMatchPlayerCoinCountsComponent::OnReleased([[maybe_unused]] float value)
    {
        const StartingPointInput::InputEventNotificationId* inputId = StartingPointInput::InputEventNotificationBus::GetCurrentBusId();

        if (inputId == nullptr)
        {
            return;
        }

        if (*inputId == ShowPlayerCoinCountsEventId)
        {
            m_onPlayerScoreChanged.Disconnect();
            UiElementBus::Event(m_rootElementId, &UiElementBus::Events::SetIsEnabled, false);
        }
    }

    PlayerNameString UiMatchPlayerCoinCountsComponent::GetPlayerName(Multiplayer::NetEntityId playerEntity)
    {
        const auto playerHandle = Multiplayer::GetNetworkEntityManager()->GetEntity(playerEntity);
        if (playerHandle.Exists())
        {
            if (const PlayerIdentityComponent* identity = playerHandle.GetEntity()->FindComponent<PlayerIdentityComponent>())
            {
                PlayerNameString playerName = identity->GetPlayerName();
                if (playerName.empty())
                {
                    return "<player_identity_empty>";
                }

                return playerName;
            }
        }

        return "<player_handle_does_not_exist>";
    }

    void UiMatchPlayerCoinCountsComponent::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<UiMatchPlayerCoinCountsComponent, AZ::Component>()
                ->Version(1)
                ->Field("Root Element", &UiMatchPlayerCoinCountsComponent::m_rootElementId)
                ->Field("Stat Rows for Players", &UiMatchPlayerCoinCountsComponent::m_playerRowElement)
                ;

            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                editContext->Class<UiMatchPlayerCoinCountsComponent>("UiMatchPlayerCoinCountsComponent",
                    "Shows list of coins collected by each player in the match.")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC_CE("CanvasUI"))

                    ->DataElement(AZ::Edit::UIHandlers::Default, &UiMatchPlayerCoinCountsComponent::m_rootElementId,
                        "Root Element",
                        "Top level element that contains all other elements for showing coins collected by players")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &UiMatchPlayerCoinCountsComponent::m_playerRowElement,
                        "Stat Rows for Players",
                        "List of rows that contains 2 text elements, one for player id and one for coin count")
                    ;
            }
        }
    }
} // namespace MultiplayerSample
