/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AzCore/Memory/SystemAllocator.h>
#include <AzCore/Module/Module.h>
#include <Unified/MPSGameLiftSystemComponent.h>


#if AZ_TRAIT_CLIENT
    #include <MatchmakingSystemComponent.h>
    #include <MPSGameLiftClientSystemComponent.h>
    #include <Components/UI/UiGameLiftConnectWithPlayerSessionData.h>
    #include <Components/UI/UiGameLiftFlexMatchConnect.h>
    #include <RegionalLatencySystemComponent.h>
#endif

 // We only want this logic to execute in dedicated server builds, not in the Editor or Unified builds.
#define AZ_DEDICATED_SERVER_ONLY (AZ_TRAIT_SERVER && !AZ_TRAIT_CLIENT)
#if AZ_DEDICATED_SERVER_ONLY
    #include <MPSGameLiftServerSystemComponent.h>
    #include <AzCore/Console/IConsole.h>
    #include <AzCore/Date/DateFormat.h>
    #include <AzCore/Settings/SettingsRegistry.h>
    #include <AzCore/Settings/SettingsRegistryMergeUtils.h>
    #include <AzCore/Platform.h>
#endif

namespace MPSGameLift
{
    class MPSGameLiftModuleInterface
        : public AZ::Module
    {
    public:
        AZ_RTTI(MPSGameLiftModuleInterface, "{2CB13C68-FA4D-4B94-8CFB-6AC71B8630B6}", AZ::Module);
        AZ_CLASS_ALLOCATOR(MPSGameLiftModuleInterface, AZ::SystemAllocator, 0);

        MPSGameLiftModuleInterface()
        {
            #if AZ_DEDICATED_SERVER_ONLY
                // Change game server logs to go into a folder based on timestamp + server process id.
                // Normally, all the server.log files would go to the same folder named "log".
                // This way GameLift can archive the logs for a specific server.

                // Timestamp
                AZ::Date::Iso8601TimestampString utcTimestampString;
                AZ::Date::GetFilenameCompatibleFormatNow(utcTimestampString);

                // Process Id
                AZStd::fixed_string<32> processIdString;
                AZStd::to_string(processIdString, AZ::Platform::GetCurrentProcessId());
                
                // Create a log subfolder using Timestamp + Process Id
                AZ::IO::FixedMaxPath projectLogPath;
                AZ::SettingsRegistry::Get()->Get(projectLogPath.Native(), AZ::SettingsRegistryMergeUtils::FilePathKey_ProjectLogPath);

                projectLogPath = projectLogPath / AZ::IO::FixedMaxPathString::format("%s_%s", utcTimestampString.c_str(), processIdString.c_str());

                AZ::SettingsRegistry::Get()->Set(AZ::SettingsRegistryMergeUtils::FilePathKey_ProjectLogPath, projectLogPath.Native());
            #endif

            m_descriptors.insert(m_descriptors.end(), {
                MPSGameLiftSystemComponent::CreateDescriptor(),
                #if AZ_TRAIT_CLIENT
                    MatchmakingSystemComponent::CreateDescriptor(),
                    RegionalLatencySystemComponent::CreateDescriptor(),
                    MPSGameLiftClientSystemComponent::CreateDescriptor(),
                    UiGameLiftConnectWithPlayerSessionData::CreateDescriptor(),
                    UiGameLiftFlexMatchConnect::CreateDescriptor(),
                #endif
                #if AZ_DEDICATED_SERVER_ONLY
                    MPSGameLiftServerSystemComponent::CreateDescriptor(),
                #endif
                });
        }

        /**
         * Add required SystemComponents to the SystemEntity.
         */
        AZ::ComponentTypeList GetRequiredSystemComponents() const override
        {
            AZ::ComponentTypeList requiredSystemComponents{
                azrtti_typeid<MPSGameLiftSystemComponent>()
            };

            #if AZ_TRAIT_CLIENT
                requiredSystemComponents.push_back(azrtti_typeid<MatchmakingSystemComponent>());
                requiredSystemComponents.push_back(azrtti_typeid<RegionalLatencySystemComponent>());
                requiredSystemComponents.push_back(azrtti_typeid<MPSGameLiftClientSystemComponent>());
            #endif

            // Only activate the MultiplayerSample GameLift server system component if this a dedicated server running on GameLift.
            #if AZ_DEDICATED_SERVER_ONLY
                const auto console = AZ::Interface<AZ::IConsole>::Get();
                if (console == nullptr)
                {
                    AZ_Assert(false, "MultiplayerSample expecting to check AZ::Console, but it's not available. Please update code to properly check if this server is running on GameLift.");
                    return requiredSystemComponents;
                }

                bool sv_gameLiftEnabled = false;
                if (console->GetCvarValue("sv_gameLiftEnabled", sv_gameLiftEnabled) != AZ::GetValueResult::Success)
                {
                    AZ_Assert(false, "MultiplayerSample expecting to access an invalid sv_gameLiftEnabled. Please update code to properly check if GameLift is enabled in order to enable it's custom GameLift server system component.")
                    return requiredSystemComponents;
                }

                if (sv_gameLiftEnabled)
                {
                    requiredSystemComponents.push_back(azrtti_typeid<MPSGameLiftServerSystemComponent>());
                }
            #endif

            return requiredSystemComponents;
        }
    };
}// namespace MPSGameLift
