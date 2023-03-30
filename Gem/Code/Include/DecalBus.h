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

        float m_scale = 1.0f;             // Scale in meters
        float m_opacity = 1.0f;           // How visible the decal is
        float m_attenutationAngle = 1.0f; // How much to attenuate based on the angle of the geometry vs the decal
        float m_lifeTime = 0.0f;          // Time until the decal begins to fade, in seconds.
        float m_fadeTime = 1.0f;          // Time it takes the decal to fade, in seconds.
        uint8_t m_sortKey = 0;            // Higher numbers sort in front of lower numbers
    };

    void SpawnDecalConfig::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serialize = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serialize->Class<MultiplayerSample::SpawnDecalConfig>()
                ->Version(0)
                ;
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