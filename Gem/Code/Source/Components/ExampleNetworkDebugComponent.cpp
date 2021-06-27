/*
 * Copyright (c) Contributors to the Open 3D Engine Project
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AzCore/Math/ToString.h>
#include <AzCore/Serialization/EditContext.h>
#include <Components/ExampleNetworkDebugComponent.h>

namespace MultiplayerSample
{
    void ExampleNetworkDebugComponent::Reflect(AZ::ReflectContext* context)
    {
        AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context);
        if (serializeContext)
        {
            serializeContext->Class<ExampleNetworkDebugComponent, AZ::Component>()
                ->Version(1);

            if (AZ::EditContext* ptrEdit = serializeContext->GetEditContext())
            {
                using namespace AZ::Edit;
                ptrEdit->Class<ExampleNetworkDebugComponent>("ExampleNetworkDebugComponent", "An example of debugging various client/server replication for an entity.")
                    ->ClassElement(ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Category, "MultiplayerSample")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC_CE("Game"))
                    ;
            }
        }
    }

    void ExampleNetworkDebugComponent::Activate()
    {
        AZ::Vector3 translation = AZ::Vector3::CreateZero();
        AZ::TransformBus::EventResult(translation, GetEntityId(), &AZ::TransformBus::Events::GetWorldTranslation);

        AZ_Printf("ExampleNetworkDebugComponent", "Entity [%s] %s spawn at %s", GetEntity()->GetName().c_str(),
            Multiplayer::GetMultiplayer()->GetAgentType() != Multiplayer::MultiplayerAgentType::Client ? "client" : "server",
            AZ::ToString(translation).c_str());

        AZ::TransformNotificationBus::Handler::BusConnect(GetEntityId());
    }

    void ExampleNetworkDebugComponent::Deactivate()
    {
        AZ::TransformNotificationBus::Handler::BusDisconnect();
    }

    void ExampleNetworkDebugComponent::OnTransformChanged([[maybe_unused]] const AZ::Transform& local, const AZ::Transform& world)
    {
        AZ_Printf("ExampleNetworkDebugComponent", "Entity [%s] %s moved to %s", GetEntity()->GetName().c_str(),
            Multiplayer::GetMultiplayer()->GetAgentType() != Multiplayer::MultiplayerAgentType::Client ? "client" : "server",
            AZ::ToString(world.GetTranslation()).c_str());
    }
}
