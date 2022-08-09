/*
 * Copyright (c) Contributors to the Open 3D Engine Project
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/Component/EntityId.h>
#include <AzCore/EBus/EBus.h>

namespace MultiplayerSample
{
    class MatchPlayerCoinsRequests : public AZ::EBusTraits
    {
    public:
        virtual ~MatchPlayerCoinsRequests() = default;

        virtual AZStd::vector<PlayerCoinState> GetPlayerCoinCounts() const = 0;
    };

    using MatchPlayerCoinsRequestBus = AZ::EBus<MatchPlayerCoinsRequests>;
}
