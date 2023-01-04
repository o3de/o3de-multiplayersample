/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <MultiplayerSampleTypes.h>
#include <PlayerIdentityBus.h>
#include <Source/AutoGen/PlayerIdentityComponent.AutoComponent.h>

namespace MultiplayerSample
{
    class PlayerIdentityComponent
        : public PlayerIdentityComponentBase
    {
    public:
        AZ_MULTIPLAYER_COMPONENT(MultiplayerSample::PlayerIdentityComponent, s_playerIdentityComponentConcreteUuid, MultiplayerSample::PlayerIdentityComponentBase);

        static void Reflect(AZ::ReflectContext* context);
        
        void OnActivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
        void OnDeactivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;

        void AssignPlayerName(const PlayerNameString& newPlayerName);
    };


    class PlayerIdentityComponentController
        : public PlayerIdentityComponentControllerBase
        , public PlayerIdentityRequestBus::Handler
    {
    public:
        explicit PlayerIdentityComponentController(PlayerIdentityComponent& parent);

        void OnActivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
        void OnDeactivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;

#if AZ_TRAIT_SERVER
        void HandleRPC_AssignPlayerName(AzNetworking::IConnection* invokingConnection, const PlayerNameString& newPlayerName) override;
        void HandleRPC_ResetPlayerState(AzNetworking::IConnection* invokingConnection, const PlayerResetOptions& resetOptions) override;
#endif

        // PlayerIdentityRequestBus overrides ...
        const char* GetPlayerIdentityName() override;
    };
}
