/*
 * Copyright (c) Contributors to the Open 3D Engine Project
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
    // This is not documented, you kind of have to jump into mcore to find this, but invalid parameter index values are max uint32_t
    // See MCORE_INVALIDINDEX32 in Gems/EMotionFX/Code/MCore/Source/Config.h
    constexpr uint32_t InvalidParamIndex = 0xFFFFFFFF;
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

        bool GetJointTransformByName(const char* a_BoneName, AZ::Transform& outJointTransform) const;
        bool GetJointTransformById(int32_t a_BoneId, AZ::Transform& outJointTransform) const;

    private:
        void OnPreRender(float deltaTime, float blendFactor);

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
        uint32_t m_velocityParamId = InvalidParamIndex;
        uint32_t m_aimTargetParamId = InvalidParamIndex;
        uint32_t m_crouchParamId = InvalidParamIndex;
        uint32_t m_aimingParamId = InvalidParamIndex;
        uint32_t m_shootParamId = InvalidParamIndex;
        uint32_t m_jumpParamId = InvalidParamIndex;
        uint32_t m_fallParamId = InvalidParamIndex;
        uint32_t m_landParamId = InvalidParamIndex;
        uint32_t m_hitParamId = InvalidParamIndex;
        uint32_t m_deathParamId = InvalidParamIndex;
    };
}
