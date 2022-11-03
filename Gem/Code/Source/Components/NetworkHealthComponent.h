/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <Source/AutoGen/NetworkHealthComponent.AutoComponent.h>

namespace MultiplayerSample
{
    class NetworkHealthComponentController
        : public NetworkHealthComponentControllerBase
    {
    public:
        NetworkHealthComponentController(NetworkHealthComponent& parent);

        void OnActivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
        void OnDeactivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;

#if AZ_TRAIT_SERVER
        void HandleSendHealthDelta(AzNetworking::IConnection* invokingConnection, const float& healthDelta) override;
#endif
    };
}
