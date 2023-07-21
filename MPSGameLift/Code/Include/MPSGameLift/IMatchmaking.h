/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */
#pragma once
#include <MPSGameLift/IRegionalLatencyFinder.h>


namespace MPSGameLift
{
    // Result data structure for a check on a matchmaking request
    struct MatchmakingResults
    {
        AZStd::string address;
        uint32_t port = 0;
        AZStd::string playerId;
        bool found = false;
    };

    // Supports matchmaking request calls to a serverless backend
    class IMatchmaking
    {
    public:
        AZ_RTTI(IMatchmaking, "{371687E5-9626-4201-91E3-0FD1F79CB8B6}");
        virtual ~IMatchmaking() = default;

        // Request a match for the player, providing player latencies for defined regions
        virtual bool RequestMatch(const RegionalLatencies& regionalLatencies) = 0;

        // Gets the current ticket id if any
        virtual AZStd::string GetTicketId() const = 0;

        // Checks if a match has been made
        // TODO: Needs to return the matchmaking results
        virtual bool HasMatch(const AZStd::string& ticketId) = 0;
    };
} // namespace MPSGameLift
