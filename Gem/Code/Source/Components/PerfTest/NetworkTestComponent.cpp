/*
 * Copyright (c) Contributors to the Open 3D Engine Project
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AzCore/Component/TransformBus.h>
#include <AzCore/Serialization/EditContext.h>
#include <Components/PerfTest/NetworkTestComponent.h>
#include <Multiplayer/IMultiplayer.h>
#include <Multiplayer/Components/NetBindComponent.h>
#include <Multiplayer/Physics/PhysicsUtils.h>

namespace MultiplayerSample
{
    void NetworkTestComponent::Reflect(AZ::ReflectContext* context)
    {
        AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context);
        if (serializeContext)
        {
            serializeContext->Class<NetworkTestComponent, AZ::Component>()
                ->Field("Enable Movement", &NetworkTestComponent::m_enableMotion)
                ->Field("Oscillator Amplitude", &NetworkTestComponent::m_oscillatorAmplitude)
                ->Field("Oscillator Speed", &NetworkTestComponent::m_oscillatorSpeedFactor)
                ->Version(1);

            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                using namespace AZ::Edit;
                editContext->Class<NetworkTestComponent>("Network Test Helper",
                    "Various helpful test tools and behaviors to test multiplayer logic and performance.")
                    ->ClassElement(ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Category, "MultiplayerSample")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC_CE("Game"))
                    ->DataElement(nullptr, &NetworkTestComponent::m_enableMotion, "Enabled", "enabled oscillation along Z axis")
                    ->DataElement(nullptr, &NetworkTestComponent::m_oscillatorAmplitude, "Oscillator Amplitude", "amplitude along Z axis")
                    ->DataElement(nullptr, &NetworkTestComponent::m_oscillatorSpeedFactor, "Oscillator Speed", "speed factor along Z axis")
                    ;
            }
        }
    }

    void NetworkTestComponent::Activate()
    {
        if (const Multiplayer::NetBindComponent* netBindComponent = GetEntity()->FindComponent<Multiplayer::NetBindComponent>())
        {
            if (netBindComponent->IsNetEntityRoleAuthority())
            {
                AZ::TickBus::Handler::BusConnect();
                m_startTranslation = GetEntity()->GetTransform()->GetWorldTranslation();
            }
        }
    }

    void NetworkTestComponent::Deactivate()
    {
        AZ::TickBus::Handler::BusDisconnect();
    }

    void NetworkTestComponent::OnTick(float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time)
    {
        m_accumulatedTime += deltaTime;

        AZ::Vector3 copy = m_startTranslation;
        copy.SetZ(copy.GetZ() + AZStd::sin(m_accumulatedTime * m_oscillatorSpeedFactor) * m_oscillatorAmplitude);
        GetEntity()->GetTransform()->SetWorldTranslation(copy);
    }
}
