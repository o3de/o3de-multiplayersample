/*
 * Copyright (c) Contributors to the Open 3D Engine Project
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/Component/Component.h>
#include <AzCore/Math/Transform.h>
#include <AzFramework/Spawnable/Spawnable.h>
#include <AzFramework/Spawnable/SpawnableEntitiesInterface.h>

namespace MultiplayerSample
{
    //! @class AttachPlayerWeaponComponent
    //! @brief Spawns a non-network gun prefab and attaches it to the player entity.
    class AttachPlayerWeaponComponent final
        : public AZ::Component
    {
    public:
        AZ_COMPONENT(MultiplayerSample::AttachPlayerWeaponComponent, "{7ebfc648-eae7-487f-b707-308ca93aeda7}");

        static void Reflect(AZ::ReflectContext* context);

        //! AZ::Component overrides.
        //! @{
        void Activate() override;
        void Deactivate() override;
        //! }@

    private:
        AZ::Data::Asset<AzFramework::Spawnable> m_gunAsset;
        AZStd::string m_boneToAttachTo;
        AZ::Transform m_attachmentTransform;

        AZStd::shared_ptr<AzFramework::EntitySpawnTicket> m_gunTicket;
    };
}
