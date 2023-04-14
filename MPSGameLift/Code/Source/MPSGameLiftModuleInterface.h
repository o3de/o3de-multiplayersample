
#include <AzCore/Memory/SystemAllocator.h>
#include <AzCore/Module/Module.h>
#include <Unified/MPSGameLiftSystemComponent.h>
#if AZ_TRAIT_CLIENT
    #include <MPSGameLiftClientSystemComponent.h>
#endif

#if AZ_TRAIT_SERVER && !AZ_TRAIT_CLIENT
    #include <MPSGameLiftServerSystemComponent.h>
    #include <AzCore/Console/IConsole.h>
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
            m_descriptors.insert(m_descriptors.end(), {
                MPSGameLiftSystemComponent::CreateDescriptor(),
                #if AZ_TRAIT_CLIENT
                    MPSGameLiftClientSystemComponent::CreateDescriptor(),
                #endif
                #if AZ_TRAIT_SERVER && !AZ_TRAIT_CLIENT
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
                requiredSystemComponents.push_back(azrtti_typeid<MPSGameLiftClientSystemComponent>());
            #endif

            // Only activate the MultiplayerSample GameLift server system component if this a dedicated server running on GameLift.
            #if (AZ_TRAIT_SERVER && !AZ_TRAIT_CLIENT)
                if (const auto console = AZ::Interface<AZ::IConsole>::Get())
                {
                    bool sv_gameLiftEnabled = false;
                    if (console->GetCvarValue("sv_gameLiftEnabled", sv_gameLiftEnabled) == AZ::GetValueResult::Success)
                    {
                        if (sv_gameLiftEnabled)
                        {
                            requiredSystemComponents.push_back(azrtti_typeid<MPSGameLiftServerSystemComponent>());
                        }
                    }
                    else
                    {
                        AZ_Assert(false, "MultiplayerSample expecting to access an invalid sv_gameLiftEnabled. Please update code to properly check if GameLift is enabled in order to enable it's custom GameLift server system component.")
                    }
                }
                else
                {
                    AZ_Assert(false, "MultiplayerSample expecting to check AZ::Console, but it's not available. Please update code to properly check if this server is running on GameLift.")
                }
            #endif

            return requiredSystemComponents;
        }
    };
}// namespace MPSGameLift
