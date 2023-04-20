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
        AZ_TYPE_INFO(GameEffect, "{E9A6959E-C52A-4BCF-907A-C880C2BD94F0}");
        static void Reflect(AZ::ReflectContext* context);

        GameEffect() = default;
        ~GameEffect();

        //! Initializes the effect.
        void Initialize();

        //! Setters for setting custom effect attributes.
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

#if AZ_TRAIT_CLIENT
        PopcornFX::StandaloneEmitter* m_emitter = nullptr;
        Audio::IAudioProxy* m_audioProxy = nullptr;
        Audio::TATLIDType m_audioTriggerId = INVALID_AUDIO_CONTROL_ID;

        PopcornFX::PopcornFXRequests* m_popcornFx = nullptr;
        Audio::IAudioSystem* m_audioSystem = nullptr;
#endif
    };
}
