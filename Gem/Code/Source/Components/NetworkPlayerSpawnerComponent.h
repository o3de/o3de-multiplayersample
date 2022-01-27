/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of
 * this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <Multiplayer/Components/NetBindComponent.h>
#include <Source/AutoGen/NetworkPlayerSpawnerComponent.AutoComponent.h>

namespace MultiplayerSample
{

    class NetworkPlayerSpawnerComponent : public NetworkPlayerSpawnerComponentBase
    {
    public:
        AZ_MULTIPLAYER_COMPONENT(
            MultiplayerSample::NetworkPlayerSpawnerComponent,
            s_networkPlayerSpawnerComponentConcreteUuid,
            MultiplayerSample::NetworkPlayerSpawnerComponentBase);

        NetworkPlayerSpawnerComponent(){};

        void OnInit() override;
        void OnActivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
        void OnDeactivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
    };
} // namespace MultiplayerSample
