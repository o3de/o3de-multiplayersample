/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <Source/AutoGen/NetworkRandomComponent.AutoComponent.h>

namespace MultiplayerSample
{
    class NetworkRandomComponentController
        : public NetworkRandomComponentControllerBase
    {
    public:
        explicit NetworkRandomComponentController(NetworkRandomComponent& parent);

        void OnActivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
        void OnDeactivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;

        uint64_t GetRandomUint64();
        int GetRandomInt();
        float GetRandomFloat();
    };
}
