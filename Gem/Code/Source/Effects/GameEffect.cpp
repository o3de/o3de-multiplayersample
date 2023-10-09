/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <Source/Effects/GameEffect.h>
#include <AzCore/Console/IConsole.h>

#if AZ_TRAIT_CLIENT
#   include <PopcornFX/PopcornFXBus.h>
#endif

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
                ->Field("ParticleAsset", &GameEffect::m_particleAssetId)
                ->Field("AudioTrigger", &GameEffect::m_audioTrigger)
                ->Field("EffectOffset", &GameEffect::m_effectOffset);

            AZ::EditContext* editContext = serializeContext->GetEditContext();
            if (editContext)
            {
                editContext->Class<GameEffect>("GameEffect", "A single game effect, consisting of a particle effect and a sound trigger pair")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &GameEffect::m_particleAssetId, "ParticleAsset", "The particle effect to play upon effect trigger")
#if AZ_TRAIT_CLIENT
                        ->Attribute(AZ_CRC_CE("SupportedAssetTypes"), []() { return AZStd::vector<AZ::Data::AssetType>({ PopcornFX::AssetTypeId }); })
#endif
                    ->DataElement(AZ::Edit::UIHandlers::Default, &GameEffect::m_audioTrigger, "AudioTrigger", "The audio trigger name of the sound to play upon effect trigger")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &GameEffect::m_effectOffset, "EffectOffset", "The offset to apply when triggering an effect");
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

        // Only copy the effect settings, but leave it in an uninitialized state. Each GameEffect instance should
        // have its own emitter to manipulate and move around.
        m_particleAssetId = gameEffect.m_particleAssetId;
        m_audioTrigger = gameEffect.m_audioTrigger;
        m_effectOffset = gameEffect.m_effectOffset;

        return *this;
    }

    void GameEffect::Destroy()
    {
#if AZ_TRAIT_CLIENT
        if (m_popcornFx && m_emitter)
        {
            if (m_popcornFx->IsEffectAlive(m_emitter))
            {
                m_popcornFx->DestroyEffect(m_emitter);
            }
        }

        if (m_audioSystem && m_audioProxy)
        {
            m_audioSystem->RecycleAudioProxy(m_audioProxy);
        }

        // Clear all of these out so that we know we need to call Initialize() again.
        m_popcornFx = nullptr;
        m_audioSystem = nullptr;
        m_emitter = nullptr;
        m_audioProxy = nullptr;
        m_audioTriggerId = INVALID_AUDIO_CONTROL_ID;
#endif
    }

    void GameEffect::Initialize([[maybe_unused]] EmitterType emitterType)
    {
#if AZ_TRAIT_CLIENT
        AZ_Assert(!IsInitialized(), "Destroy() needs to be called before calling Initialize() for a second time.");
        if (IsInitialized())
        {
            return;
        }

        m_popcornFx = PopcornFX::PopcornFXRequestBus::FindFirstHandler();
        m_audioSystem = AZ::Interface<Audio::IAudioSystem>::Get();
        m_emitterType = emitterType;

        if (m_popcornFx != nullptr)
        {
            if (m_particleAssetId.IsValid())
            {
                if (m_emitterType == EmitterType::ReusableEmitter)
                {
                    // Spawn the emitter in a disabled state, and set it to always exist (i.e. don't auto-remove).
                    // GameEffect will keep reusing the same emitter.
                    constexpr bool EffectEnabled = false;
                    constexpr bool AutoRemove = false;
                    const PopcornFX::SpawnParams params = PopcornFX::SpawnParams(EffectEnabled, AutoRemove, AZ::Transform::CreateIdentity());
                    m_emitter = m_popcornFx->SpawnEffectById(m_particleAssetId, params);
                }
                else
                {
                    // Don't spawn anything, just preload the particle asset.
                    m_popcornFx->PreloadEffectById(m_particleAssetId);
                }
            }
        }

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
#if AZ_TRAIT_CLIENT
        return (m_popcornFx && m_emitter);
#else
        return true;
#endif
    }

    bool GameEffect::SetAttribute([[maybe_unused]] const char* attributeName, [[maybe_unused]] float value) const
    {
#if AZ_TRAIT_CLIENT
        AZ_Assert(m_emitterType == EmitterType::ReusableEmitter, "SetAttribute only supports reusable emitters.");
        if (m_popcornFx && m_emitter)
        {
            if (m_popcornFx->IsEffectAlive(m_emitter))
            {
                int32_t attrId = m_popcornFx->EffectGetAttributeId(m_emitter, attributeName);
                if (attrId >= 0)
                {
                    return m_popcornFx->EffectSetAttributeAsFloat(m_emitter, attrId, value);
                }
            }
            else
            {
                AZ_Assert(false, "Setting attribute on an emitter that isn't active.");
                return false;
            }
        }
#endif
        return false;
    }

    bool GameEffect::SetAttribute([[maybe_unused]] const char* attributeName, [[maybe_unused]] const AZ::Vector2& value) const
    {
#if AZ_TRAIT_CLIENT
        AZ_Assert(m_emitterType == EmitterType::ReusableEmitter, "SetAttribute only supports reusable emitters.");
        if (m_popcornFx && m_emitter)
        {
            if (m_popcornFx->IsEffectAlive(m_emitter))
            {
                int32_t attrId = m_popcornFx->EffectGetAttributeId(m_emitter, attributeName);
                if (attrId >= 0)
                {
                    return m_popcornFx->EffectSetAttributeAsFloat2(m_emitter, attrId, value);
                }
            }
            else
            {
                AZ_Assert(false, "Setting attribute on an emitter that isn't active.");
                return false;
            }
        }
#endif
        return false;
    }

    bool GameEffect::SetAttribute([[maybe_unused]] const char* attributeName, [[maybe_unused]] const AZ::Vector3& value) const
    {
#if AZ_TRAIT_CLIENT
        AZ_Assert(m_emitterType == EmitterType::ReusableEmitter, "SetAttribute only supports reusable emitters.");
        if (m_popcornFx && m_emitter)
        {
            if (m_popcornFx->IsEffectAlive(m_emitter))
            {
                int32_t attrId = m_popcornFx->EffectGetAttributeId(m_emitter, attributeName);
                if (attrId >= 0)
                {
                    return m_popcornFx->EffectSetAttributeAsFloat3(m_emitter, attrId, value);
                }
            }
            else
            {
                AZ_Assert(false, "Setting attribute on an emitter that isn't active.");
                return false;
            }
        }
#endif
        return false;
    }

    bool GameEffect::SetAttribute([[maybe_unused]] const char* attributeName, [[maybe_unused]] const AZ::Vector4& value) const
    {
#if AZ_TRAIT_CLIENT
        AZ_Assert(m_emitterType == EmitterType::ReusableEmitter, "SetAttribute only supports reusable emitters.");
        if (m_popcornFx && m_emitter)
        {
            if (m_popcornFx->IsEffectAlive(m_emitter))
            {
                int32_t attrId = m_popcornFx->EffectGetAttributeId(m_emitter, attributeName);
                if (attrId >= 0)
                {
                    return m_popcornFx->EffectSetAttributeAsFloat4(m_emitter, attrId, value);
                }
            }
            else
            {
                AZ_Assert(false, "Setting attribute on an emitter that isn't active.");
                return false;
            }
        }
#endif
        return false;
    }

    void GameEffect::TriggerEffect([[maybe_unused]] const AZ::Transform& transform) const
    {
#if AZ_TRAIT_CLIENT
        const AZ::Vector3 offsetPosition = transform.TransformPoint(m_effectOffset);

        if (m_popcornFx)
        {
            AZ::Transform transformOffset = transform;
            transformOffset.SetTranslation(offsetPosition);

            if (m_emitterType == EmitterType::ReusableEmitter)
            {
                if (m_emitter && m_popcornFx->IsEffectAlive(m_emitter))
                {
                    m_popcornFx->EffectSetTransform(m_emitter, transformOffset);
                    m_popcornFx->EffectSetTeleportThisFrame(m_emitter);

                    // It's important to do this *after* setting the transform. Otherwise, on the first time we trigger the effect,
                    // it will spawn the effect briefly at (0, 0, 0) before moving and restarting it.

                    m_popcornFx->EffectEnable(m_emitter, true);

                    // EffectRestart will either Kill or Terminate the effect on restart.
                    m_popcornFx->EffectRestart(m_emitter, cl_KillEffectOnRestart);
                }
                else
                {
                    AZ_Assert(false, "Triggering an inactive emitter.");
                }
            }
            else
            {
                // For fire-and-forget, spawn a new effect on each call to TriggerEffect(), and set it to auto-remove.
                // We won't track it, and it will continue to run even if this GameEffect instance gets destroyed.
                constexpr bool EffectEnabled = true;
                constexpr bool AutoRemove = true;
                const PopcornFX::SpawnParams params = PopcornFX::SpawnParams(EffectEnabled, AutoRemove, transformOffset);
                m_popcornFx->SpawnEffectById(m_particleAssetId, params);
            }
        }

        if ((m_audioProxy != nullptr) && (m_audioTriggerId != INVALID_AUDIO_CONTROL_ID))
        {
            m_audioProxy->SetPosition(offsetPosition);
            m_audioProxy->ExecuteTrigger(m_audioTriggerId);
        }
#endif
    }

    void GameEffect::StopEffect() const
    {
#if AZ_TRAIT_CLIENT
        if (m_popcornFx && m_emitter)
        {
            if (m_popcornFx->IsEffectAlive(m_emitter))
            {
                m_popcornFx->EffectKill(m_emitter);
            }
        }
#endif
    }

    const AZ::Vector3& GameEffect::GetEffectOffset() const
    {
        return m_effectOffset;
    }
}
