/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/Math/Transform.h>
#include <AzCore/EBus/EBus.h>
#include <Atom/RPI.Public/Scene.h>

namespace MultiplayerSample
{
    struct SpawnDecalConfig
    {
        AZ_RTTI(MultiplayerSample::SpawnDecalConfig, "{FC3DA616-174B-48FD-9BFB-BC277132FB47}");
        inline static void Reflect(AZ::ReflectContext* context);

        AZ::Data::AssetId m_materialAssetId; // Asset Id of the material.
        float m_scale = 1.0f;                // Scale in meters.
        float m_opacity = 1.0f;              // How visible the decal is.
        float m_attenuationAngle = 1.0f;     // How much to attenuate based on the angle of the geometry vs the decal.
        float m_lifeTimeSec = 0.0f;          // Length of time the decal lives between fading in and out, in seconds.
        float m_fadeInTimeSec = 0.1f;        // Time it takes the decal to fade in, in seconds.
        float m_fadeOutTimeSec = 1.0f;       // Time it takes the decal to fade out, in seconds.
        float m_thickness = 1.0f;            // How thick the decal should be on the z axis.
        uint8_t m_sortKey = 0;               // Higher numbers sort in front of lower numbers.
    };

    void SpawnDecalConfig::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<MultiplayerSample::SpawnDecalConfig>()
                ->Version(0)
                ->Field("MaterialAssetId", &SpawnDecalConfig::m_materialAssetId)
                ->Field("Scale", &SpawnDecalConfig::m_scale)
                ->Field("Opacity", &SpawnDecalConfig::m_opacity)
                ->Field("AttenuationAngle", &SpawnDecalConfig::m_attenuationAngle)
                ->Field("LifeTimeSec", &SpawnDecalConfig::m_lifeTimeSec)
                ->Field("FadeInTimeSec", &SpawnDecalConfig::m_fadeInTimeSec)
                ->Field("FadeOutTimeSec", &SpawnDecalConfig::m_fadeOutTimeSec)
                ->Field("Thickness", &SpawnDecalConfig::m_thickness)
                ->Field("SortKey", &SpawnDecalConfig::m_sortKey)
                ;

            if (auto editContext = serializeContext->GetEditContext())
            {
                editContext->Class<SpawnDecalConfig>("SpawnDecalConfig", "Configuration settings for spawning a decal.")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &SpawnDecalConfig::m_materialAssetId, "Material", "The material for the decal.")
                    ->DataElement(AZ::Edit::UIHandlers::Slider, &SpawnDecalConfig::m_scale, "Scale", "The scale of the decal.")
                        ->Attribute(AZ::Edit::Attributes::Min, 0.01f)
                        ->Attribute(AZ::Edit::Attributes::Max, 100.0f)
                        ->Attribute(AZ::Edit::Attributes::SoftMin, 0.01f)
                        ->Attribute(AZ::Edit::Attributes::SoftMax, 5.0f)
                    ->DataElement(AZ::Edit::UIHandlers::Slider, &SpawnDecalConfig::m_opacity, "Opacity", "The opacity of the decal.")
                        ->Attribute(AZ::Edit::Attributes::Min, 0.0f)
                        ->Attribute(AZ::Edit::Attributes::Max, 1.0f)
                    ->DataElement(AZ::Edit::UIHandlers::Slider, &SpawnDecalConfig::m_attenuationAngle, "Angle attenuation", "How much to attenuate the opacity of the decal based on the different in the angle between the decal and the surface.")
                        ->Attribute(AZ::Edit::Attributes::Min, 0.0f)
                        ->Attribute(AZ::Edit::Attributes::Max, 1.0f)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &SpawnDecalConfig::m_lifeTimeSec, "Life time", "Length of time the decal lives between fading in and out, in seconds")
                        ->Attribute(AZ::Edit::Attributes::Min, 0.0f)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &SpawnDecalConfig::m_fadeInTimeSec, "Fade in time", "How long the decal should spend fading in when it is first spawned, in seconds.")
                        ->Attribute(AZ::Edit::Attributes::Min, 0.0f)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &SpawnDecalConfig::m_fadeOutTimeSec, "Fade out time", "How long the decal should spend fading out at the end of its life time, in seconds.")
                        ->Attribute(AZ::Edit::Attributes::Min, 0.0f)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &SpawnDecalConfig::m_thickness, "Thickness", "How thick the decal should be on the z axis.")
                        ->Attribute(AZ::Edit::Attributes::Min, 0.0f)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &SpawnDecalConfig::m_sortKey, "Sort key", "Used to sort the decal with other decals. Higher numbered decals show on top of lower number decals.")
                    ;
            }
        }

        if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            behaviorContext->Class<SpawnDecalConfig>("SpawnDecalConfig")
                ->Attribute(AZ::Script::Attributes::Scope, AZ::Script::Attributes::ScopeFlags::Common)
                ->Attribute(AZ::Script::Attributes::Category, "Graphics")
                ->Attribute(AZ::Script::Attributes::Module, "decals")
                ->Constructor()
                ->Constructor<const SpawnDecalConfig&>()
                ->Property("material", BehaviorValueProperty(&SpawnDecalConfig::m_materialAssetId))
                ->Property("scale", BehaviorValueProperty(&SpawnDecalConfig::m_scale))
                ->Property("opacity", BehaviorValueProperty(&SpawnDecalConfig::m_opacity))
                ->Property("attenuationAngle", BehaviorValueProperty(&SpawnDecalConfig::m_attenuationAngle))
                ->Property("lifeTimeSec", BehaviorValueProperty(&SpawnDecalConfig::m_lifeTimeSec))
                ->Property("fadeInTimeSec", BehaviorValueProperty(&SpawnDecalConfig::m_fadeInTimeSec))
                ->Property("fadeOutTimeSec", BehaviorValueProperty(&SpawnDecalConfig::m_fadeOutTimeSec))
                ->Property("thickness", BehaviorValueProperty(&SpawnDecalConfig::m_thickness))
                ->Property("sortKey", BehaviorValueProperty(&SpawnDecalConfig::m_sortKey))
                ;
        }
    }

    class DecalRequests
        : public AZ::EBusTraits
    {
    public:
        static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::ById;
        using BusIdType = AZ::RPI::SceneId;

        AZ_RTTI(MultiplayerSample::DecalRequests, "{A3643473-25ED-4F86-8BEE-D65A1A54867B}");
        virtual ~DecalRequests() = default;

        /**
         * \brief Spawn a decal
         * \param worldTm Where to spawn the decal.
         * \param materialAssetId The asset ID of the material to use for the decal
         * \param config The configuration of the decal to spawn (opacity, scale, etc).
         */
        virtual void SpawnDecal(const AZ::Transform& worldTm, const SpawnDecalConfig& config) = 0;
    };

    using DecalRequestBus = AZ::EBus<DecalRequests>;
}
