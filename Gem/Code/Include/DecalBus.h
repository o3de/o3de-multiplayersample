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

        float m_scale = 1.0f;             // Scale in meters.
        float m_opacity = 1.0f;           // How visible the decal is.
        float m_attenutationAngle = 1.0f; // How much to attenuate based on the angle of the geometry vs the decal.
        float m_lifeTime = 0.0f;          // Time until the decal begins to fade, in seconds.
        float m_fadeTime = 1.0f;          // Time it takes the decal to fade, in seconds.
        float m_thickness = 1.0f;         // How thick the decal should be on the z axis.
        uint8_t m_sortKey = 0;            // Higher numbers sort in front of lower numbers.
    };

    void SpawnDecalConfig::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<MultiplayerSample::SpawnDecalConfig>()
                ->Version(0)
                ->Field("Scale", &SpawnDecalConfig::m_scale)
                ->Field("Opacity", &SpawnDecalConfig::m_opacity)
                ->Field("AttenuationAngle", &SpawnDecalConfig::m_attenutationAngle)
                ->Field("LifeTime", &SpawnDecalConfig::m_lifeTime)
                ->Field("FadeTime", &SpawnDecalConfig::m_fadeTime)
                ->Field("Thickness", &SpawnDecalConfig::m_thickness)
                ->Field("SortKey", &SpawnDecalConfig::m_sortKey)
                ;

            if (auto editContext = serializeContext->GetEditContext())
            {
                editContext->Class<SpawnDecalConfig>("SpawnDecalConfig", "Configuration settings for spawning a decal.")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &SpawnDecalConfig::m_scale, "Scale", "The scale of the decal.")
                    ->DataElement(AZ::Edit::UIHandlers::Slider, &SpawnDecalConfig::m_opacity, "Opacity", "The opacity of the decal.")
                        ->Attribute(AZ::Edit::Attributes::Min, 0.0f)
                        ->Attribute(AZ::Edit::Attributes::Max, 1.0f)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &SpawnDecalConfig::m_attenutationAngle, "Angle attenuation", "How much to attenuate the opacity of the decal based on the different in the angle between the decal and the surface.")
                        ->Attribute(AZ::Edit::Attributes::Min, 0.0f)
                        ->Attribute(AZ::Edit::Attributes::Max, 1.0f)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &SpawnDecalConfig::m_lifeTime, "Life time", "How long before the decal should begin to fade out, in seconds.")
                        ->Attribute(AZ::Edit::Attributes::Min, 0.0f)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &SpawnDecalConfig::m_fadeTime, "Fade time", "How long the decal should spend fading out at the end of its life time.")
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
                ->Property("scale", BehaviorValueProperty(&SpawnDecalConfig::m_scale))
                ->Property("opacity", BehaviorValueProperty(&SpawnDecalConfig::m_opacity))
                ->Property("m_attenutationAngle", BehaviorValueProperty(&SpawnDecalConfig::m_attenutationAngle))
                ->Property("m_lifeTime", BehaviorValueProperty(&SpawnDecalConfig::m_lifeTime))
                ->Property("m_fadeTime", BehaviorValueProperty(&SpawnDecalConfig::m_fadeTime))
                ->Property("m_thickness", BehaviorValueProperty(&SpawnDecalConfig::m_thickness))
                ->Property("m_sortKey", BehaviorValueProperty(&SpawnDecalConfig::m_sortKey))
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
        virtual void SpawnDecal(const AZ::Transform& worldTm, AZ::Data::AssetId materialAssetId, const SpawnDecalConfig& config) = 0;
    };

    using DecalRequestBus = AZ::EBus<DecalRequests>;
}