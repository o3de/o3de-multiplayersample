/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 * 
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include "MultiplayerSampleSystemComponent.h"

#include <AzCore/Console/ILogger.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/EditContextConstants.inl>

#include <Source/AutoGen/AutoComponentTypes.h>
#include <Source/Weapons/WeaponTypes.h>
#include <Source/Components/NetworkStressTestComponent.h>
#include <Source/Components/NetworkAiComponent.h>
#include <Source/Effects/GameEffect.h>
#include <Source/UserSettings/MultiplayerSampleUserSettings.h>
#include <Multiplayer/Components/NetBindComponent.h>

#include <AzFramework/Scene/Scene.h>
#include <Atom/RPI.Public/Scene.h>

namespace MultiplayerSample
{
    using namespace AzNetworking;

    void MultiplayerSampleSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        ReflectWeaponEnums(context);
        GatherParams::Reflect(context);
        HitEffect::Reflect(context);
        HitEntity::Reflect(context);
        HitEvent::Reflect(context);
        WeaponParams::Reflect(context);
        GameEffect::Reflect(context);

        GemSpawnable::Reflect(context);
        GemWeightChance::Reflect(context);
        RoundSpawnTable::Reflect(context);

        if (AZ::SerializeContext* serialize = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serialize->Class<MultiplayerSampleSystemComponent, AZ::Component>()
                ->Version(0);

            if (AZ::EditContext* ec = serialize->GetEditContext())
            {
                ec->Class<MultiplayerSampleSystemComponent>("MultiplayerSample", "[Description of functionality provided by this System Component]")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("System"))
                        ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ;
            }
        }

        if (AZ::BehaviorContext* behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            // This will put these methods into the 'azlmbr.atomtools.general' module
            auto addGeneral = [](AZ::BehaviorContext::GlobalMethodBuilder methodBuilder)
            {
                methodBuilder->Attribute(AZ::Script::Attributes::Scope, AZ::Script::Attributes::ScopeFlags::Common)
                    ->Attribute(AZ::Script::Attributes::Category, "MultiplayerSample")
                    ->Attribute(AZ::Script::Attributes::Module, "MultiplayerSample.general");
            };

            addGeneral(behaviorContext->Method(
                "GetRenderSceneIdByName", &MultiplayerSampleSystemComponent::GetRenderSceneIdByName, nullptr,
                "Gets an RPI scene ID based on the name of the AzFramework Scene."));
        }
    }

    void MultiplayerSampleSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("MultiplayerSampleService"));
    }

    void MultiplayerSampleSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("MultiplayerSampleService"));
    }

    void MultiplayerSampleSystemComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        required.push_back(AZ_CRC_CE("NetworkingService"));
        required.push_back(AZ_CRC_CE("MultiplayerService"));
    }

    void MultiplayerSampleSystemComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        // We're dependent on this to start first so that we can apply the MSAA setting at the correct point in the boot process.
        // If we're ever able to apply the MSAA setting at runtime, this can get removed, along with the call to ApplyMsaaSetting().
        dependent.push_back(AZ_CRC_CE("AzFrameworkConfigurationSystemComponentService"));
    }

    void MultiplayerSampleSystemComponent::Init()
    {
        ;
    }

    void MultiplayerSampleSystemComponent::Activate()
    {
        //! Register our gems multiplayer components to assign NetComponentIds
        RegisterMultiplayerComponents();

        // Tell the user settings that this is the correct point in the boot process to apply the MSAA setting.
        MultiplayerSampleUserSettingsRequestBus::Broadcast(
            &MultiplayerSampleUserSettingsRequestBus::Events::ApplyMsaaSetting);
    }

    void MultiplayerSampleSystemComponent::Deactivate()
    {
    }

    AZ::Uuid MultiplayerSampleSystemComponent::GetRenderSceneIdByName(const AZStd::string& name)
    {
        AZStd::shared_ptr<AzFramework::Scene> scene = AzFramework::SceneSystemInterface::Get()->GetScene(name);
        if (scene)
        {
            AZ::RPI::ScenePtr renderScene = *scene->FindSubsystemInScene<AZ::RPI::ScenePtr>();
            if (renderScene)
            {
                return renderScene->GetId();
            }
        }
        return AZ::Uuid::CreateInvalid();
    }
}

