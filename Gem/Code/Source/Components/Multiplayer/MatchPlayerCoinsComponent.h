/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <MatchPlayerCoinsBus.h>
#include <PlayerCoinCollectorBus.h>
#include <Source/AutoGen/MatchPlayerCoinsComponent.AutoComponent.h>

namespace MultiplayerSample
{
    class MatchPlayerCoinsComponent
        : public MatchPlayerCoinsComponentBase
        , public MatchPlayerCoinsRequestBus::Handler
    {
    public:
        AZ_MULTIPLAYER_COMPONENT(MultiplayerSample::MatchPlayerCoinsComponent, s_matchPlayerCoinsComponentConcreteUuid, MultiplayerSample::MatchPlayerCoinsComponentBase);

        static void Reflect(AZ::ReflectContext* context);
        
        void OnActivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
        void OnDeactivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;

        //! MatchPlayerCoinsRequestBus overrides ...
        //! @{
        AZStd::vector<PlayerCoinState> GetPlayerCoinCounts() const override;
        //! }@
    };

    class MatchPlayerCoinsComponentController
        : public MatchPlayerCoinsComponentControllerBase
        , public PlayerCoinCollectorNotificationBus::Handler
    {
    public:
        explicit MatchPlayerCoinsComponentController(MatchPlayerCoinsComponent& parent);

        void OnActivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
        void OnDeactivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;

        void ResetAllCoins();

        //! PlayerCoinCollectorNotificationBus overrides ...
        //! @{
        void OnPlayerCollectedCoinCountChanged(Multiplayer::NetEntityId playerEntity, uint16_t coinsCollected) override;
        void OnPlayerCollectorActivated(Multiplayer::NetEntityId playerEntity) override;
        void OnPlayerCollectorDeactivated(Multiplayer::NetEntityId playerEntity) override;
        //! }@

    private:
        // Return -1 if a state is not available for this player id
        int GetCoinStateIndex(Multiplayer::NetEntityId playerEntity) const;
    };
}
