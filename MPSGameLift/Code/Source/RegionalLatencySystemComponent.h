/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once


#include <MPSGameLift/IRegionalLatencyFinder.h>

#include <AzCore/Component/Component.h>
#include <AzCore/EBus/ScheduledEvent.h>
#include <AzCore/RTTI/BehaviorContext.h>
#include <AzCore/std/containers/vector.h>

namespace MPSGameLift
{
    class RegionalLatencySystemComponent final
        : public AZ::Component
        , public IRegionalLatencyFinder
    {
    public:
        static constexpr char RegionalEndpointUrlFormat[] = "https://dynamodb.%s.amazonaws.com";
        static constexpr const char* Regions[] = { "us-west-2", "us-east-1" };


        AZ_COMPONENT(RegionalLatencySystemComponent, "{699E7875-5274-4516-88C9-A8D3010B9D3A}");

        /*
        * Reflects component data into the reflection contexts, including the serialization, edit, and behavior contexts.
        */
        static void Reflect(AZ::ReflectContext* context);
        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);

        /* IRegionalLatencyFinder overrides... */
        void RequestLatencies() override;
        void AddRequestLatenciesCompleteEventHandler(RequestLatenciesCompleteEvent::Handler& handler) override;
        AZStd::chrono::milliseconds GetLatencyForRegion(const AZStd::string& region) const override;
   
    protected:
        void Activate() override;
        void Deactivate() override;

    private:
        AZStd::atomic_int m_responsesPending = 0;
        mutable AZStd::mutex m_mapMutex;
        RegionalLatencies m_regionalLatencies;
        RequestLatenciesCompleteEvent m_requestLatenciesCompleteEvent;

        AZ::ScheduledEvent m_broadcastLatencyCompleteMainThread{ [this]()
        {
            AZStd::lock_guard<AZStd::mutex> lock(m_mapMutex);
            m_requestLatenciesCompleteEvent.Signal(m_regionalLatencies);
        }, AZ::Name("BroadcastLatencyComplete") };
    };
} // namespace MPSGameLift
