/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 * 
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <Source/AutoGen/NetworkAnimationComponent.AutoComponent.h>
#include <Multiplayer/Components/NetBindComponent.h>
#include <Integration/ActorComponentBus.h>
#include <Integration/AnimGraphComponentBus.h>

namespace EMotionFX
{
    class AnimGraphComponentNetworkRequests;
    namespace Integration
    {
        class ActorComponentRequests;
        class AnimGraphComponentRequests;
    }
}

namespace MultiplayerSample
{
    // This is not documented, you kind of have to jump into EMotionFX's private headers to find this, invalid parameter index values are max size_t
    // See InvalidIndex in Gems\EMotionFX\Code\EMotionFX\Source\EMotionFXConfig.h
    constexpr size_t InvalidParamIndex = 0xffffffffffffffff;
    constexpr int32_t  InvalidBoneId = -1;

    class NetworkAnimationComponent
        : public NetworkAnimationComponentBase
        , private EMotionFX::Integration::ActorComponentNotificationBus::Handler
        , private EMotionFX::Integration::AnimGraphComponentNotificationBus::Handler
    {
    public:
        AZ_MULTIPLAYER_COMPONENT(MultiplayerSample::NetworkAnimationComponent, s_networkAnimationComponentConcreteUuid, MultiplayerSample::NetworkAnimationComponentBase);

        static void Reflect(AZ::ReflectContext* context);

        NetworkAnimationComponent();

        void OnInit() override;
        void OnActivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
        void OnDeactivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;

        int32_t GetBoneIdByName(const char* boneName) const;

        bool GetJointTransformByName(const char* boneName, AZ::Transform& outJointTransform) const;
        bool GetJointTransformById(int32_t boneId, AZ::Transform& outJointTransform) const;

    private:
        void OnPreRender(float deltaTime);

        //! EMotionFX::Integration::ActorComponentNotificationBus::Handler
        //! @{
        void OnActorInstanceCreated(EMotionFX::ActorInstance* actorInstance) override;
        void OnActorInstanceDestroyed(EMotionFX::ActorInstance* actorInstance) override;
        //! @}

        //! EMotionFX::Integration::AnimGraphComponentNotificationBus::Handler
        //! @{
        void OnAnimGraphInstanceCreated(EMotionFX::AnimGraphInstance* animGraphInstance) override;
        //! @}

        Multiplayer::EntityPreRenderEvent::Handler m_preRenderEventHandler;

        EMotionFX::Integration::ActorComponentRequests* m_actorRequests = nullptr;
        EMotionFX::AnimGraphComponentNetworkRequests* m_networkRequests = nullptr;
        EMotionFX::Integration::AnimGraphComponentRequests* m_animationGraph = nullptr;

        // Hardcoded parameters, be nice if this was flexible and configurable from within the editor
        size_t m_movementDirectionParamId = InvalidParamIndex;
        size_t m_movementSpeedParamId = InvalidParamIndex;
        size_t m_velocityParamId = InvalidParamIndex;
        size_t m_aimTargetParamId = InvalidParamIndex;
        size_t m_crouchParamId = InvalidParamIndex;
        size_t m_aimingParamId = InvalidParamIndex;
        size_t m_shootParamId = InvalidParamIndex;
        size_t m_jumpParamId = InvalidParamIndex;
        size_t m_fallParamId = InvalidParamIndex;
        size_t m_landParamId = InvalidParamIndex;
        size_t m_hitParamId = InvalidParamIndex;
        size_t m_deathParamId = InvalidParamIndex;
    };
}
