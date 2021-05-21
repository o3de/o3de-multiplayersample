/*
 * All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
 * its licensors.
 *
 * For complete copyright and license terms please see the LICENSE at the root of this
 * distribution (the "License"). All use of this software is governed by the License,
 * or, if provided, by the license below or the license accompanying this file. Do not
 * remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *
 */

#include <Source/Components/NetworkAnimationComponent.h>
#include <Source/Components/WasdPlayerMovementComponent.h>
#include <Integration/AnimGraphComponentBus.h>
#include <Integration/AnimationBus.h>

namespace MultiplayerSample
{
    void NetworkAnimationComponent::NetworkAnimationComponent::Reflect(AZ::ReflectContext* context)
    {
        AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context);
        if (serializeContext)
        {
            serializeContext->Class<NetworkAnimationComponent, NetworkAnimationComponentBase>()
                ->Version(1);
        }
        NetworkAnimationComponentBase::Reflect(context);
    }

    void NetworkAnimationComponent::OnInit()
    {
        ;
    }

    void NetworkAnimationComponent::OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        AZ::TickBus::Handler::BusConnect();

        m_actorRequests = EMotionFX::Integration::ActorComponentRequestBus::FindFirstHandler(GetEntityId());
        m_animationGraph = EMotionFX::Integration::AnimGraphComponentRequestBus::FindFirstHandler(GetEntityId());

        if (m_animationGraph != nullptr)
        {
            m_movementSpeedParamId = m_animationGraph->FindParameterIndex(GetMovementSpeedParamName().c_str());
            m_attackParamId = m_animationGraph->FindParameterIndex(GetAttackParamName().c_str());
            m_jumpParamId = m_animationGraph->FindParameterIndex(GetJumpParamName().c_str());
        }

        EMotionFX::Integration::ActorComponentNotificationBus::Handler::BusConnect(GetEntityId());
    }

    void NetworkAnimationComponent::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        AZ::TickBus::Handler::BusDisconnect();
        EMotionFX::Integration::ActorComponentNotificationBus::Handler::BusDisconnect();
    }

    int32_t NetworkAnimationComponent::GetBoneIdByName(const char* boneName) const
    {
        if (m_actorRequests != nullptr)
        {
            return m_actorRequests->GetJointIndexByName(boneName);
        }
        return InvalidBoneId;
    }

    bool NetworkAnimationComponent::GetJointTransformByName(const char* jointName, AZ::Transform& outJointTransform) const
    {
        if (m_actorRequests == nullptr)
        {
            return false;
        }
        const int32_t jointId = m_actorRequests->GetJointIndexByName(jointName);
        return GetJointTransformById(jointId, outJointTransform);
    }

    bool NetworkAnimationComponent::GetJointTransformById(int32_t jointId, AZ::Transform& outJointTransform) const
    {
        if ((m_actorRequests == nullptr) || (jointId == InvalidBoneId))
        {
            return false;
        }
        outJointTransform = m_actorRequests->GetJointTransform(jointId, EMotionFX::Integration::Space::WorldSpace);
        return true;
    }

    void NetworkAnimationComponent::OnTick([[maybe_unused]] float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time)
    {
        if ((m_animationGraph != nullptr) && (m_movementSpeedParamId == InvalidParamIndex))
        {
            m_movementSpeedParamId = m_animationGraph->FindParameterIndex(GetMovementSpeedParamName().c_str());
            m_attackParamId = m_animationGraph->FindParameterIndex(GetAttackParamName().c_str());
            m_jumpParamId = m_animationGraph->FindParameterIndex(GetJumpParamName().c_str());
        }

        if (m_movementSpeedParamId != InvalidParamIndex)
        {
            const float speed = GetWasdPlayerMovementComponent()->GetSpeed() / GetMaxSpeed();
            m_animationGraph->SetParameterFloat(m_movementSpeedParamId, speed);
        }

        if (m_attackParamId != InvalidParamIndex)
        {
            const bool attacking = GetActiveAnimStates().GetBit(aznumeric_cast<AZStd::size_t>(CharacterAnimState::Attacking));
            m_animationGraph->SetParameterBool(m_attackParamId, attacking);
        }

        if (m_jumpParamId != InvalidParamIndex)
        {
            const bool jumping = GetActiveAnimStates().GetBit(aznumeric_cast<AZStd::size_t>(CharacterAnimState::Jumping));
            m_animationGraph->SetParameterBool(m_jumpParamId, jumping);
        }
    }

    int NetworkAnimationComponent::GetTickOrder()
    {
        // It is quite critical that the network transform component updates before the network animation component
        // That means, if you touch this, and server anims are hooked up, you should triple check that tick ordering is still correct
        // We're also assuming EMotionFX will eventually use AZ::TICK_ANIMATION, so we're pre-emptively setting ourselves to run before
        return AZ::TICK_ANIMATION - 1;
    }

    void NetworkAnimationComponent::OnActorInstanceCreated([[maybe_unused]] EMotionFX::ActorInstance* actorInstance)
    {
        m_actorRequests = EMotionFX::Integration::ActorComponentRequestBus::FindFirstHandler(GetEntityId());
    }

    void NetworkAnimationComponent::OnActorInstanceDestroyed([[maybe_unused]] EMotionFX::ActorInstance* actorInstance)
    {
        m_actorRequests = nullptr;
    }
}
