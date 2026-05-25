/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <Source/Effects/GameEffect.h>
#include <AzCore/Console/IConsole.h>

namespace MultiplayerSample
{
    AZ_CVAR(bool, cl_KillEffectOnRestart, false, nullptr, AZ::ConsoleFunctorFlags::Null, "Controls whether to kill or terminate current effects on restart");

    void GameEffect::Reflect(AZ::ReflectContext* context)
    {
        AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context);
        if (serializeContext)
        {
            serializeContext->Class<GameEffect>()
                ->Version(1) 
                // TODO:  OpenParticleSystem_MPS -  When we get to the point of being
                // able to reference Particle Assets directly, we will add them here.
                // ->Field("ParticleAsset", &GameEffect::m_particleAsset)
                ->Field("AudioTrigger", &GameEffect::m_audioTrigger)
                ->Field("EffectOffset", &GameEffect::m_effectOffset);
                
            AZ::EditContext* editContext = serializeContext->GetEditContext();
            if (editContext)
            {
                editContext->Class<GameEffect>("GameEffect", "A single game effect, consisting of a particle effect and a sound trigger pair")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    // TODO:  OpenParticleSystem_MPS -  When we get to the point of being
                    // able to reference Particle Assets directly, we will add them here.
                    //->DataElement(AZ::Edit::UIHandlers::Default, &GameEffect::m_particleAsset, "Particle Asset", "The particle effect to spawn when triggered")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &GameEffect::m_audioTrigger, "AudioTrigger", "The audio trigger name of the sound to play upon effect trigger")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &GameEffect::m_effectOffset, "EffectOffset", "The offset to apply when triggering an effect")
					;
            }
        }
    }

    GameEffect::~GameEffect()
    {
        Destroy();
    }

    GameEffect::GameEffect(const GameEffect& gameEffect)
    {
        *this = gameEffect;
    }

    GameEffect& GameEffect::operator=(const GameEffect& gameEffect)
    {
        // Make sure the current emitter is destroyed before copying new settings over this one.
        Destroy();

        // TODO:  OpenParticleSystem_MPS
        // m_particleAsset = gameEffect.m_particleAsset;
        m_audioTrigger = gameEffect.m_audioTrigger;
        m_effectOffset = gameEffect.m_effectOffset;

        return *this;
    }

    void GameEffect::Destroy()
    {
        if (!IsInitialized())
        {
            return;
		}

        m_isInitialized = false;

#if AZ_TRAIT_CLIENT
        // TODO:  OpenParticleSystem_MPS - Destroy the OpenParticleSystem emitter here.
        if (m_audioSystem && m_audioProxy)
        {
            m_audioSystem->RecycleAudioProxy(m_audioProxy);
        }

        m_audioSystem = nullptr;
        m_audioProxy = nullptr;
        m_audioTriggerId = INVALID_AUDIO_CONTROL_ID;
#endif // AZ_TRAIT_CLIENT
    }

    void GameEffect::Initialize([[maybe_unused]] EmitterType emitterType)
    {
#if AZ_TRAIT_CLIENT
        AZ_Assert(!IsInitialized(), "Destroy() needs to be called before calling Initialize() for a second time.");
        if (IsInitialized())
        {
            return;
        }
        m_isInitialized = true;
        m_audioSystem = AZ::Interface<Audio::IAudioSystem>::Get();
        m_emitterType = emitterType;

        // TODO:  OpenParticleSystem_MPS - Initialize the OpenParticleSystem effect here.
        if (m_audioSystem != nullptr)
        {
            m_audioProxy = m_audioSystem->GetAudioProxy();
            m_audioProxy->Initialize(m_audioTrigger.c_str(), this);
            m_audioProxy->SetObstructionCalcType(Audio::ObstructionType::Ignore);
            m_audioTriggerId = m_audioSystem->GetAudioTriggerID(m_audioTrigger.c_str());
        }
#endif
    }

    bool GameEffect::IsInitialized() const
    {
        return m_isInitialized;
    }

    bool GameEffect::SetAttribute([[maybe_unused]] const char* attributeName, [[maybe_unused]] float value) const
    {
#if AZ_TRAIT_CLIENT
    AZ_Assert(m_emitterType == EmitterType::ReusableEmitter, "SetAttribute only supports reusable emitters.");
    // TODO:  OpenParticleSystem_MPS - Send attributes to the OpenParticleSystem emitter here.
    #endif // AZ_TRAIT_CLIENT
        return false;
    }

    bool GameEffect::SetAttribute([[maybe_unused]] const char* attributeName, [[maybe_unused]] const AZ::Vector2& value) const
    {
#if AZ_TRAIT_CLIENT
    AZ_Assert(m_emitterType == EmitterType::ReusableEmitter, "SetAttribute only supports reusable emitters.");
    // TODO:  OpenParticleSystem_MPS - Send attributes to the OpenParticleSystem emitter here.
#endif //   AZ_TRAIT_CLIENT 
        return false;
    }

    bool GameEffect::SetAttribute([[maybe_unused]] const char* attributeName, [[maybe_unused]] const AZ::Vector3& value) const
    {
#if AZ_TRAIT_CLIENT
        AZ_Assert(m_emitterType == EmitterType::ReusableEmitter, "SetAttribute only supports reusable emitters.");
        // TODO:  OpenParticleSystem_MPS - Send attributes to the OpenParticleSystem emitter here.
#endif
        return false;
    }

    bool GameEffect::SetAttribute([[maybe_unused]] const char* attributeName, [[maybe_unused]] const AZ::Vector4& value) const
    {
#if AZ_TRAIT_CLIENT
    AZ_Assert(m_emitterType == EmitterType::ReusableEmitter, "SetAttribute only supports reusable emitters.");
    // TODO:  OpenParticleSystem_MPS - Send attributes to the OpenParticleSystem emitter here.
#endif
        return false;
    }

    void GameEffect::TriggerEffect([[maybe_unused]] const AZ::Transform& transform) const
    {
#if AZ_TRAIT_CLIENT
        // const AZ::Vector3 offsetPosition = transform.TransformPoint(m_effectOffset);
        // TODO:  OpenParticleSystem_MPS - Trigger the OpenParticleSystem Effect here.
#endif // AZ_TRAIT_CLIENT
    }

    void GameEffect::StopEffect() const
    {
#if AZ_TRAIT_CLIENT
        // TODO:  OpenParticleSystem_MPS - Stop the OpenParticleSystem effect here.
#endif
    }

    const AZ::Vector3& GameEffect::GetEffectOffset() const
    {
        return m_effectOffset;
    }
}
