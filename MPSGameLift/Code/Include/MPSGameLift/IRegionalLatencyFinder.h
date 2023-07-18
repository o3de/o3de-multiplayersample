/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/RTTI/RTTIMacros.h>
#include <AzCore/std/chrono/chrono.h>
#include <AzCore/std/string/string.h>


namespace MPSGameLift
{
    typedef AZStd::unordered_map<AZStd::string, AZStd::chrono::milliseconds> RegionalLatencies;
    using RequestLatenciesCompleteEvent = AZ::Event<const RegionalLatencies&>;

    //! @class IRegionalLatencyFinder
    //! @brief IRegionalLatencyFinder provides estimate information about the network latency between server regions and this client application.
    //!   Example: This interface requests and stores the round-trip-time it takes for the game client to reach a server endpoint
    //!            on the East Coast versus the West Coast United States.
    //!
    //! IRegionalLatencyFinder is an AZ::Interface<T> that provides applications access to
    //! regional server latency information.  IRegionalLatencyFinder is implemented on the
    //! RegionLatencySystemComponent.
    class IRegionalLatencyFinder
    {
    public:
        AZ_RTTI(IRegionalLatencyFinder, "{D2171936-1BC5-44B9-BC49-9666A829ED17}");

        virtual ~IRegionalLatencyFinder() = default;

        // Sends an HTTP request to gather the latency for all set regions
        virtual void RequestLatencies() = 0;

        // Adds a RequestLatenciesCompleteEvent Handler which is invoked when all RequestLatencies() HTTP requests receive a response (or time out).
        // @param handler The RequestLatenciesCompleteEvent Handler to add
        virtual void AddRequestLatenciesCompleteEventHandler(RequestLatenciesCompleteEvent::Handler& handler) = 0;

        // Gets the measured latency for a given region
        // @param Region (example: us-west-2)
        // @return The round-trip-time of sending and receiving a response from a given regional endpoint 
        virtual AZStd::chrono::milliseconds GetLatencyForRegion(const AZStd::string& region) const = 0;
    };
} //namespace MPSGameLift
