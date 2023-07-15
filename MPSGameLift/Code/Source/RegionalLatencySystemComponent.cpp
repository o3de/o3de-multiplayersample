/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include "RegionalLatencySystemComponent.h"

#include <aws/core/http/HttpResponse.h>
#include <HttpRequestor/HttpRequestorBus.h>
#include <HttpRequestor/HttpTypes.h>

#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/RTTI/BehaviorContext.h>


namespace MPSGameLift
{
    void RegionalLatencySystemComponent::Activate()
    {
        AZ::Interface<IRegionalLatencyFinder>::Register(this);
    }

    void RegionalLatencySystemComponent::Deactivate()
    {
        AZ::Interface<IRegionalLatencyFinder>::Unregister(this);
    }

    void RegionalLatencySystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (const auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<RegionalLatencySystemComponent, AZ::Component>()
                ->Version(1)
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
                        m_regionalLatencies[region] = roundTripTime;
                    }
                    else
                    {
                        AZ_Error("RegionalLatencySystemComponent", false, "Failed to receive response for region: %s.", region);
                    }

                    // Check if all requests have responded
                    m_responsesPending.fetch_sub(1);
                    if (m_responsesPending.load() == 0)
                    {
                        RegionalLatencyFinderNotificationBus::Broadcast(&RegionalLatencyFinderNotifications::OnRequestLatenciesComplete, m_regionalLatencies);
                    }
                });
        }
    }

    AZStd::chrono::milliseconds RegionalLatencySystemComponent::GetLatencyForRegion(const AZStd::string& region) const
    {
        {
            AZStd::lock_guard<AZStd::mutex> lock(m_mapMutex);
            if (const auto latencyKeyValue = m_regionalLatencies.find(region); latencyKeyValue != m_regionalLatencies.end())
            {
                return latencyKeyValue->second;
            }
        }

        AZ_Warning("RegionalLatencySystemComponent", false, "GetLatencyForRegion failed for region %s. Did you forget to first call RequestLatencies?", region.c_str());
        return {};
    }
} // namespace MPSGameLift
