
#include <Utils/RegionalLatencySystemComponent.h>

#include <aws/core/http/HttpResponse.h>
#include <HttpRequestor/HttpRequestorBus.h>
#include <HttpRequestor/HttpTypes.h>

#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/RTTI/BehaviorContext.h>

namespace MPSGameLift
{
    RegionalLatencySystemComponent::RegionalLatencySystemComponent()
    {
        AZ::Interface<IRegionalLatencyFinder>::Register(this);

    }

    RegionalLatencySystemComponent::~RegionalLatencySystemComponent()
    {
        AZ::Interface<IRegionalLatencyFinder>::Unregister(this);
    }

    void RegionalLatencySystemComponent::Activate()
    {
    }

    void RegionalLatencySystemComponent::Deactivate()
    {
    }

    void RegionalLatencySystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (const auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<RegionalLatencySystemComponent, AZ::Component>()
                ->Version(1)
            ;
        }

        if (AZ::BehaviorContext* behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            behaviorContext->EBus<RegionalLatencyFinderNotificationBus>("RegionalLatencyFinderNotificationBus")->
                Handler<MPSGameLift::BehaviorRegionalLatencyFinderBusHandler>();


            behaviorContext->Class<RegionalLatencySystemComponent>("RegionalLatencySystemComponent")
                ->Attribute(AZ::Script::Attributes::Category, "MPSGameLift Gem")
                ->Attribute(AZ::Script::Attributes::Scope, AZ::Script::Attributes::ScopeFlags::Common)

                ->Method("RequestLatencies", []()
                    {
                        if (const auto latencyFinder = AZ::Interface<IRegionalLatencyFinder>::Get())
                        {
                            latencyFinder->RequestLatencies();
                        }
                    })
                ->Method("GetLatencyForRegionMs", [](const AZStd::string& region) -> uint32_t
                    {
                        if (const auto latencyFinder = AZ::Interface<IRegionalLatencyFinder>::Get())
                        {
                            return aznumeric_cast<uint32_t>(latencyFinder->GetLatencyForRegion(region).count());
                        }

                        return 0;
                    })
            ;
        }
    }

    void RegionalLatencySystemComponent::RequestLatencies()
    {
        // Response callbacks are received on a separate thread. 
        // Users can call RequestLatencies from main thread multiple times before all the requests
        // have returned a response. For simplicity, don't make more requests if requests are still waiting for a response.
        if (m_responsesPending.load() > 0)
        {
            AZ_Warning("RegionalLatencySystemComponent", false, 
                "Denying RequestLatencies. "
                "Latencies are already being received; "
                "listen for RegionalLatencyFinderNotifications::OnRequestLatenciesComplete to get a notification once all region latencies have been received.");
            return;
        }

        // Start pinging region endpoints
        m_responsesPending.store(aznumeric_cast<int>(AZStd::size(Regions)));

        for (auto region : Regions)
        {
            AZStd::string regionEndpoint = AZStd::string::format(RegionalEndpointUrlFormat, region);

            HttpRequestor::HttpRequestorRequestBus::Broadcast(&HttpRequestor::HttpRequestorRequests::AddTextRequest, regionEndpoint, Aws::Http::HttpMethod::HTTP_GET,
                [this, region]([[maybe_unused]]const AZStd::string& response, Aws::Http::HttpResponseCode responseCode)
                {
                    if (responseCode == Aws::Http::HttpResponseCode::OK)
                    {
                        AZStd::chrono::milliseconds roundTripTime;
                        HttpRequestor::HttpRequestorRequestBus::BroadcastResult(roundTripTime, &HttpRequestor::HttpRequestorRequests::GetLastRoundTripTime);

                        AZStd::lock_guard<AZStd::mutex> lock(m_mapMutex);
                        m_latencyMap[region] = roundTripTime;
                    }
                    else
                    {
                        AZ_Error("RegionalLatencySystemComponent", false, "Failed to receive response for region: %s.", region);
                    }

                    // Check if all requests have responded
                    m_responsesPending.fetch_sub(1);
                    if (m_responsesPending.load() == 0)
                    {
                        RegionalLatencyFinderNotificationBus::Broadcast(&RegionalLatencyFinderNotifications::OnRequestLatenciesComplete);
                    }
                });
        }
    }

    AZStd::chrono::milliseconds RegionalLatencySystemComponent::GetLatencyForRegion(const AZStd::string& region) const
    {
        {
            AZStd::lock_guard<AZStd::mutex> lock(m_mapMutex);
            if (const auto latencyKeyValue = m_latencyMap.find(region); latencyKeyValue != m_latencyMap.end())
            {
                return latencyKeyValue->second;
            }
        }

        AZ_Warning("RegionalLatencySystemComponent", false, "GetLatencyForRegion failed for region %s. Did you forget to first call RequestLatencies?", region.c_str());
        return {};
    }
} // namespace MPSGameLift

