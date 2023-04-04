/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <PlayerIdentityBus.h>
#include <Source/Components/NetworkHealthComponent.h>
#include <Source/Components/Multiplayer/PlayerCoinCollectorComponent.h>
#include <Source/Components/Multiplayer/PlayerIdentityComponent.h>

#if AZ_TRAIT_CLIENT
    #include <Atom/RPI.Public/ViewportContextBus.h>
    #include <AzFramework/Viewport/ViewportScreen.h>
    #include <AzCore/Component/TransformBus.h>
#endif

namespace MultiplayerSample
{
    void PlayerIdentityComponent::Reflect(AZ::ReflectContext* context)
    {
        AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context);
        if (serializeContext)
        {
            serializeContext->Class<PlayerIdentityComponent, PlayerIdentityComponentBase>()
                ->Version(1);
        }
        PlayerIdentityComponentBase::Reflect(context);
    }

    void PlayerIdentityComponent::OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        #if AZ_TRAIT_CLIENT
            m_viewport = AZ::RPI::ViewportContextRequests::Get()->GetDefaultViewportContext();
            if (!m_viewport)
            {
                AZ_Assert(false, "NetworkDebugPlayerIdComponent failed to find the a rendering viewport. Debug rendering will be disabled.");
                return;
            }

            const auto fontQueryInterface = AZ::Interface<AzFramework::FontQueryInterface>::Get();
            if (!fontQueryInterface)
            {
                AZ_Assert(false, "NetworkDebugPlayerIdComponent failed to find the FontQueryInterface. Debug rendering will be disabled.");
                return;
            }

            m_fontDrawInterface = fontQueryInterface->GetDefaultFontDrawInterface();
            if (!m_fontDrawInterface)
            {
                AZ_Assert(false, "NetworkDebugPlayerIdComponent failed to find the FontDrawInterface. Debug rendering will be disabled.");
                return;
            }

            m_drawParams.m_drawViewportId = m_viewport->GetId();
            m_drawParams.m_scale = AZ::Vector2(FontScale);
            m_drawParams.m_color = AZ::Colors::Wheat;

            AZ::TickBus::Handler::BusConnect();
        #endif
    }

    void PlayerIdentityComponent::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        #if AZ_TRAIT_CLIENT
            AZ::TickBus::Handler::BusDisconnect();
        #endif
    }

#if AZ_TRAIT_CLIENT
    void PlayerIdentityComponent::OnTick([[maybe_unused]] float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time)
    {
        // Don't render others players' on-screen debug text if the player is behind the camera
        const AZ::Vector3 renderWorldSpace = GetEntity()->GetTransform()->GetWorldTranslation();
        if (!IsNetEntityRoleAutonomous())
        {
            AZ::Vector3 cameraForward = m_viewport->GetCameraTransform().GetBasisY();
            AZ::Vector3 cameraToPlayer = renderWorldSpace - m_viewport->GetCameraTransform().GetTranslation();
            if (cameraForward.Dot(cameraToPlayer) < 0.0f)
            {
                return;
            }
        }

        const AzFramework::WindowSize windowSize = m_viewport->GetViewportSize();
        AzFramework::ScreenPoint renderScreenpoint = AzFramework::WorldToScreen(
            renderWorldSpace, m_viewport->GetCameraViewMatrixAsMatrix3x4(), m_viewport->GetCameraProjectionMatrix(), AzFramework::ScreenSize(windowSize.m_width, windowSize.m_height));
        m_drawParams.m_hAlign = AzFramework::TextHorizontalAlignment::Center;
        m_drawParams.m_position = AZ::Vector3(aznumeric_cast<float>(renderScreenpoint.m_x), aznumeric_cast<float>(renderScreenpoint.m_y), 0.f);

        m_fontDrawInterface->DrawScreenAlignedText2d(m_drawParams, GetPlayerName().c_str());
    }
#endif

    PlayerIdentityComponentController::PlayerIdentityComponentController(PlayerIdentityComponent& parent)
        : PlayerIdentityComponentControllerBase(parent)
    {
    }

    void PlayerIdentityComponentController::OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        if (IsNetEntityRoleAuthority())
        {
            PlayerIdentityNotificationBus::Broadcast(&PlayerIdentityNotificationBus::Events::OnPlayerActivated, GetNetEntityId());
        }

        if (IsNetEntityRoleAutonomous())
        {
            PlayerNameAddEvent(m_onAutomonousPlayerNameChanged);
            PlayerIdentityRequestBus::Handler::BusConnect();
        }
    }

    void PlayerIdentityComponentController::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        if (IsNetEntityRoleAuthority())
        {
            PlayerIdentityNotificationBus::Broadcast(&PlayerIdentityNotificationBus::Events::OnPlayerDeactivated, GetNetEntityId());
        }

        PlayerIdentityRequestBus::Handler::BusDisconnect();
        m_onAutomonousPlayerNameChanged.Disconnect();
    }

#if AZ_TRAIT_SERVER
    void PlayerIdentityComponentController::HandleRPC_AssignPlayerName([[maybe_unused]] AzNetworking::IConnection* invokingConnection,
        const PlayerNameString& newPlayerName)
    {
        SetPlayerName(newPlayerName);
    }

    void PlayerIdentityComponentController::HandleRPC_ResetPlayerState([[maybe_unused]] AzNetworking::IConnection* invokingConnection, const PlayerResetOptions& resetOptions)
    {
        if (resetOptions.m_resetArmor)
        {
            GetNetworkHealthComponentController()->SetHealth(GetNetworkHealthComponentController()->GetMaxHealth());
        }
        auto currentCoins = GetPlayerCoinCollectorComponentController()->GetCoinsCollected();
        float coinsToDeduct = currentCoins * (resetOptions.m_coinPenalty * 0.01f);

        GetPlayerCoinCollectorComponentController()->SetCoinsCollected(currentCoins - aznumeric_cast<uint16_t>(coinsToDeduct));
        PlayerIdentityComponentControllerBase::HandleRPC_ResetPlayerState(invokingConnection, resetOptions);
    }
#endif

    const char* PlayerIdentityComponentController::GetPlayerIdentityName()
    {
        return GetParent().GetPlayerName().c_str();
    }
}
