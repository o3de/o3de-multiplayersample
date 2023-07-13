#pragma once

#include <AzCore/Component/ComponentBus.h>

namespace MPSGameLift
{
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

        // Request latency checks for all set regions
        virtual void RequestLatencies() = 0;

        // Gets the measured latency for a given AWS region
        virtual AZStd::chrono::milliseconds GetLatencyForRegion(const AZStd::string& region) const = 0;
    };

    class RegionalLatencyFinderNotifications
        : public AZ::ComponentBus
    {
    public:
        using MutexType = AZStd::recursive_mutex;

        virtual void OnRequestLatenciesComplete() {}
    };
    typedef AZ::EBus<RegionalLatencyFinderNotifications> RegionalLatencyFinderNotificationBus;

} //namespace MPSGameLift
