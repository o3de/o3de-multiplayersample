
#pragma once

#include <MPSGameLift/IRegionalLatencyFinder.h>

#include <AzCore/Component/Component.h>
#include <AzCore/RTTI/BehaviorContext.h>
#include <AzCore/std/containers/vector.h>

namespace MPSGameLift
{
    class BehaviorRegionalLatencyFinderBusHandler
        : public RegionalLatencyFinderNotificationBus::Handler
        , public AZ::BehaviorEBusHandler
    {
    public: 
        AZ_EBUS_BEHAVIOR_BINDER
        (
            BehaviorRegionalLatencyFinderBusHandler, "{BC890DD0-4430-4497-864D-8F694B208EAF}", AZ::SystemAllocator,
            OnRequestLatenciesComplete
        );

        void OnRequestLatenciesComplete() override
        {
            Call(FN_OnRequestLatenciesComplete);
        }
    };

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

        RegionalLatencySystemComponent();
        ~RegionalLatencySystemComponent() override;

        /* IRegionalLatencyFinder overrides... */
        void RequestLatencies() override;
        AZStd::chrono::milliseconds GetLatencyForRegion(const AZStd::string& region) const override;
   
    protected:
        void Activate() override;
        void Deactivate() override;

    private:
        AZStd::atomic_int m_responsesPending = 0;
        mutable AZStd::mutex m_mapMutex;
        AZStd::unordered_map<AZStd::string, AZStd::chrono::milliseconds> m_latencyMap;
    };
} // namespace MPSGameLift
