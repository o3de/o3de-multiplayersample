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
    AZ_CVAR(bool, cl_KillEffectOnRestart, false, nullptr, AZ::ConsoleFunctorFlags::Null, "Controls whether or not to kill current effects on restart");

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
#if AZ_TRAIT_CLIENT
        if (m_popcornFx != nullptr)
        {
            m_popcornFx->DestroyEffect(m_emitter);
            m_emitter = nullptr;
        }

        if (m_audioSystem != nullptr)
        {
            m_audioTriggerId = INVALID_AUDIO_CONTROL_ID;
            if (m_audioProxy != nullptr)
            {
                m_audioSystem->RecycleAudioProxy(m_audioProxy);
                m_audioProxy = nullptr;
            }
        }
#endif
    }

    void GameEffect::Initialize()
    {
#if AZ_TRAIT_CLIENT
        m_popcornFx = PopcornFX::PopcornFXRequestBus::FindFirstHandler();
        m_audioSystem = AZ::Interface<Audio::IAudioSystem>::Get();

        if (m_popcornFx != nullptr)
        {
            const PopcornFX::SpawnParams params = PopcornFX::SpawnParams(true, false, AZ::Transform::CreateIdentity());
            m_emitter = m_popcornFx->SpawnEffectById(m_particleAssetId, params);
        }

        if (m_audioSystem != nullptr)
        {
            m_audioProxy = m_audioSystem->GetAudioProxy();
            m_audioTriggerId = m_audioSystem->GetAudioTriggerID(m_audioTrigger.c_str());
        }
#endif
    }

    bool GameEffect::SetAttribute([[maybe_unused]] const char* attributeName, [[maybe_unused]] float value) const
    {
#if AZ_TRAIT_CLIENT
        if (m_popcornFx != nullptr)
        {
            int32_t attrId = m_popcornFx->EffectGetAttributeId(m_emitter, attributeName);
            if (attrId >= 0)
            {
                return m_popcornFx->EffectSetAttributeAsFloat(m_emitter, attrId, value);
            }
        }
#endif
        return false;
    }

    bool GameEffect::SetAttribute([[maybe_unused]] const char* attributeName, [[maybe_unused]] const AZ::Vector2& value) const
    {
#if AZ_TRAIT_CLIENT
        if (m_popcornFx != nullptr)
        {
            int32_t attrId = m_popcornFx->EffectGetAttributeId(m_emitter, attributeName);
            if (attrId >= 0)
            {
                return m_popcornFx->EffectSetAttributeAsFloat2(m_emitter, attrId, value);
            }
        }
#endif
        return false;
    }

    bool GameEffect::SetAttribute([[maybe_unused]] const char* attributeName, [[maybe_unused]] const AZ::Vector3& value) const
    {
#if AZ_TRAIT_CLIENT
        if (m_popcornFx != nullptr)
        {
            int32_t attrId = m_popcornFx->EffectGetAttributeId(m_emitter, attributeName);
            if (attrId >= 0)
            {
                return m_popcornFx->EffectSetAttributeAsFloat3(m_emitter, attrId, value);
            }
        }
#endif
        return false;
    }

    bool GameEffect::SetAttribute([[maybe_unused]] const char* attributeName, [[maybe_unused]] const AZ::Vector4& value) const
    {
#if AZ_TRAIT_CLIENT
        if (m_popcornFx != nullptr)
        {
            int32_t attrId = m_popcornFx->EffectGetAttributeId(m_emitter, attributeName);
            if (attrId >= 0)
            {
                return m_popcornFx->EffectSetAttributeAsFloat4(m_emitter, attrId, value);
            }
        }
#endif
        return false;
    }

    void GameEffect::TriggerEffect([[maybe_unused]] const AZ::Transform& transform) const
    {
#if AZ_TRAIT_CLIENT
        const AZ::Vector3 offsetPosition = transform.GetTranslation() + m_effectOffset;
        AZ::Transform transformOffset = transform;
        transformOffset.SetTranslation(offsetPosition);
        if (m_emitter != nullptr)
        {
            if (PopcornFX::PopcornFXRequests* popcornFx = PopcornFX::PopcornFXRequestBus::FindFirstHandler())
            {
                popcornFx->EffectSetTransform(m_emitter, transformOffset);
                popcornFx->EffectRestart(m_emitter, cl_KillEffectOnRestart);
            }
        }

        if ((m_audioProxy != nullptr) && (m_audioTriggerId != INVALID_AUDIO_CONTROL_ID))
        {
            m_audioProxy->SetPosition(offsetPosition);
            m_audioProxy->ExecuteTrigger(m_audioTriggerId);
        }
#endif
    }
}
