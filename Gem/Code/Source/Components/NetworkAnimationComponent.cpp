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
#include <Source/Components/CharacterComponent.h>
#include <Source/Components/SimplePlayerCameraComponent.h>
#include <Source/Components/WasdPlayerMovementComponent.h>
#include <Integration/AnimGraphComponentBus.h>
#include <Integration/AnimationBus.h>
#include <Integration/AnimGraphNetworkingBus.h>
#include <AzCore/Component/TransformBus.h>

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
        m_networkRequests = EMotionFX::AnimGraphComponentNetworkRequestBus::FindFirstHandler(GetEntityId());
        m_animationGraph = EMotionFX::Integration::AnimGraphComponentRequestBus::FindFirstHandler(GetEntityId());

        EMotionFX::Integration::ActorComponentNotificationBus::Handler::BusConnect(GetEntityId());
        EMotionFX::Integration::AnimGraphComponentNotificationBus::Handler::BusConnect(GetEntityId());
    }

    void NetworkAnimationComponent::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        EMotionFX::Integration::ActorComponentNotificationBus::Handler::BusDisconnect();
        AZ::TickBus::Handler::BusDisconnect();
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
        if (m_animationGraph == nullptr || m_networkRequests == nullptr)
        {
            return;
        }

        m_networkRequests->UpdateActorExternal(deltaTime);

        if (m_velocityParamId == InvalidParamIndex)
        {
            m_velocityParamId = m_animationGraph->FindParameterIndex(GetVelocityParamName().c_str());
            m_aimTargetParamId = m_animationGraph->FindParameterIndex(GetAimTargetParamName().c_str());
            m_crouchParamId = m_animationGraph->FindParameterIndex(GetCrouchParamName().c_str());
            m_aimingParamId = m_animationGraph->FindParameterIndex(GetAimingParamName().c_str());
            m_shootParamId = m_animationGraph->FindParameterIndex(GetShootParamName().c_str());
            m_jumpParamId = m_animationGraph->FindParameterIndex(GetJumpParamName().c_str());
            m_fallParamId = m_animationGraph->FindParameterIndex(GetFallParamName().c_str());
            m_landParamId = m_animationGraph->FindParameterIndex(GetLandParamName().c_str());
            m_hitParamId = m_animationGraph->FindParameterIndex(GetHitParamName().c_str());
            m_deathParamId = m_animationGraph->FindParameterIndex(GetDeathParamName().c_str());
        }

        if (m_velocityParamId != InvalidParamIndex)
        {
            WasdPlayerMovementComponentController* controller = static_cast<WasdPlayerMovementComponentController*>(GetWasdPlayerMovementComponent()->GetController());
            if (controller)
            {
                const AZ::Vector3 velocity = controller->GetVelocity();
                const AZ::Vector2 velocity2d = AZ::Vector2(velocity.GetX(), velocity.GetY());
                const float maxSpeed = GetCharacterComponent()->GetSprintSpeed();
                m_animationGraph->SetParameterVector2(m_velocityParamId, velocity2d / maxSpeed);
            }
        }

        if (m_aimTargetParamId != InvalidParamIndex)
        {
            const AZ::Vector3 aimAngles = GetSimplePlayerCameraComponent()->GetAimAngles();
            const AZ::Quaternion aimRotation = AZ::Quaternion::CreateRotationZ(aimAngles.GetZ()) * AZ::Quaternion::CreateRotationX(aimAngles.GetX());
            const AZ::Transform worldTm = GetEntity()->GetTransform()->GetWorldTM();
            // TODO: This should probably be a physx raycast out to some maxDistance
            const AZ::Vector3 aimTarget = worldTm.GetTranslation() + aimRotation.TransformVector(AZ::Vector3(5.0f));
            m_animationGraph->SetParameterVector3(m_aimTargetParamId, aimTarget);
        }

        if (m_crouchParamId != InvalidParamIndex)
        {
            const bool crouching = GetActiveAnimStates().GetBit(aznumeric_cast<AZStd::size_t>(CharacterAnimState::Crouching));
            m_animationGraph->SetParameterBool(m_crouchParamId, crouching);
        }

        if (m_aimingParamId != InvalidParamIndex)
        {
            const bool aiming = GetActiveAnimStates().GetBit(aznumeric_cast<AZStd::size_t>(CharacterAnimState::Aiming));
            m_animationGraph->SetParameterBool(m_aimingParamId, aiming);
        }

        if (m_shootParamId != InvalidParamIndex)
        {
            const bool shooting = GetActiveAnimStates().GetBit(aznumeric_cast<AZStd::size_t>(CharacterAnimState::Shooting));
            m_animationGraph->SetParameterBool(m_shootParamId, shooting);
        }

        if (m_jumpParamId != InvalidParamIndex)
        {
            const bool jumping = GetActiveAnimStates().GetBit(aznumeric_cast<AZStd::size_t>(CharacterAnimState::Jumping));
            m_animationGraph->SetParameterBool(m_jumpParamId, jumping);
        }

        if (m_fallParamId != InvalidParamIndex)
        {
            const bool falling = GetActiveAnimStates().GetBit(aznumeric_cast<AZStd::size_t>(CharacterAnimState::Falling));
            m_animationGraph->SetParameterBool(m_fallParamId, falling);
        }

        if (m_landParamId != InvalidParamIndex)
        {
            const bool landing = GetActiveAnimStates().GetBit(aznumeric_cast<AZStd::size_t>(CharacterAnimState::Landing));
            m_animationGraph->SetParameterBool(m_landParamId, landing);
        }

        if (m_hitParamId != InvalidParamIndex)
        {
            const bool hit = GetActiveAnimStates().GetBit(aznumeric_cast<AZStd::size_t>(CharacterAnimState::Hit));
            m_animationGraph->SetParameterBool(m_hitParamId, hit);
        }

        if (m_deathParamId != InvalidParamIndex)
        {
            const bool dead = GetActiveAnimStates().GetBit(aznumeric_cast<AZStd::size_t>(CharacterAnimState::Dying));
            m_animationGraph->SetParameterBool(m_deathParamId, dead);
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

    void NetworkAnimationComponent::OnAnimGraphInstanceCreated([[maybe_unused]] EMotionFX::AnimGraphInstance* animGraphInstance)
    {
        // We don't need any more notifications
        EMotionFX::Integration::AnimGraphComponentNotificationBus::Handler::BusDisconnect();

        // Disable automatic EMotionFX updates of transform, network has control
        if (m_networkRequests != nullptr)
        {
            constexpr bool isAuthoritative = true;
            m_networkRequests->CreateSnapshot(isAuthoritative);
        }
    }
}
