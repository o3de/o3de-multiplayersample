/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/Asset/AssetCommon.h>
#include <AzCore/Asset/AssetSerializer.h>
#include <AzCore/std/string/fixed_string.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Math/Transform.h>

#if AZ_TRAIT_CLIENT
#   include <IAudioSystem.h>
#endif

namespace PopcornFX
{
    struct StandaloneEmitter;
    class PopcornFXRequests;
}

namespace MultiplayerSample
{
    //! A class containing a singular game effect, consisting of a graphical particle effect and an attached sound trigger.
    class GameEffect final
    {
    public:
        enum class EmitterType
        {
            // Fire-and-forget emitters will create a new emitter on each TriggerEffect and won't be tracked by GameEffect,
            // so they will always run until they're done and then get auto-removed by PopcornFX.
            FireAndForget,

            // Reusable emitters create a single emitter per GameEffect and will reuse the emitter on each TriggerEffect.
            // The emitter will be immediately destroyed when the GameEffect is destroyed.
            ReusableEmitter
        };
        
        AZ_TYPE_INFO(GameEffect, "{E9A6959E-C52A-4BCF-907A-C880C2BD94F0}");
        static void Reflect(AZ::ReflectContext* context);

        GameEffect() = default;
        GameEffect(const GameEffect& gameEffect);
        GameEffect& operator=(const GameEffect& gameEffect);
        ~GameEffect();

        //! Initializes the effect emitter.
        void Initialize(EmitterType emitterType = EmitterType::ReusableEmitter);

        //! Destroys the effect emitter;
        void Destroy();

        //! True if the effect is initialized, false if it isn't.
        bool IsInitialized() const;

        //! Setters for setting custom effect attributes.
        //! These only work for reusable emitters because we don't track the emitter pointer for fire-and-forget emitters.
        //! @{
        bool SetAttribute(const char* attributeName, float value) const;
        bool SetAttribute(const char* attributeName, const AZ::Vector2& value) const;
        bool SetAttribute(const char* attributeName, const AZ::Vector3& value) const;
        bool SetAttribute(const char* attributeName, const AZ::Vector4& value) const;
        //! @}

        //! Triggers the attached effect at the provided transform.
        //! @param transform the root transform to move the effect to prior to triggering
        void TriggerEffect(const AZ::Transform& transform) const;

        //! Stops the attached effect if it's executing.
        void StopEffect() const;

        //! Returns the configured effect offset.
        //! @return the effect offset
        const AZ::Vector3& GetEffectOffset() const;

    private:
        AZ::Data::AssetId m_particleAssetId; // The particle effect to play upon effect activation
        AZStd::string m_audioTrigger; // The name of the audio trigger to use on effect activation
        AZ::Vector3 m_effectOffset = AZ::Vector3::CreateZero(); // The offset to use when triggering an effect

        // Tracks whether to reuse the emitter or to fire-and-forget each effect trigger.
        [[maybe_unused]] EmitterType m_emitterType = EmitterType::ReusableEmitter; 

#if AZ_TRAIT_CLIENT
        PopcornFX::StandaloneEmitter* m_emitter = nullptr;
        Audio::IAudioProxy* m_audioProxy = nullptr;
        Audio::TATLIDType m_audioTriggerId = INVALID_AUDIO_CONTROL_ID;

        PopcornFX::PopcornFXRequests* m_popcornFx = nullptr;
        Audio::IAudioSystem* m_audioSystem = nullptr;
#endif
    };
}
