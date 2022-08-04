/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzFramework/Physics/Common/PhysicsEvents.h>
#include <Source/AutoGen/PlayerCoinCollectorComponent.AutoComponent.h>

namespace MultiplayerSample
{
    class PlayerCoinCollectorComponentController
        : public PlayerCoinCollectorComponentControllerBase
    {
    public:
        explicit PlayerCoinCollectorComponentController(PlayerCoinCollectorComponent& parent);

        void OnActivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
        void OnDeactivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;

    private:
        void OnTriggerEvents(const AzPhysics::TriggerEventList& tel);
        AzPhysics::SceneEvents::OnSceneTriggersEvent::Handler m_trigger{[this](
            AzPhysics::SceneHandle, const AzPhysics::TriggerEventList& tel)
        {
            OnTriggerEvents(tel);
        }};
    };
}
