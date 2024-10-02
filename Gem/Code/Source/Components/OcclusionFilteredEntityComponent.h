/*
 * Copyright (c) Contributors to the Open 3D Engine Project
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/Component/Component.h>
#include <Multiplayer/NetworkEntity/IFilterEntityManager.h>

namespace MultiplayerSample
{
    //! @class OcclusionFilteredEntityComponent
    //! @brief An example of using IFilterEntityManager that filters occluded player entities out of network replication.
    class OcclusionFilteredEntityComponent final
        : public AZ::Component
        , public Multiplayer::IFilterEntityManager
    {
    public:
        AZ_COMPONENT(MultiplayerSample::OcclusionFilteredEntityComponent, "{D1C628E6-F165-47F4-9C12-7BA96A80DF50}");

        static void Reflect(AZ::ReflectContext* context);

        //! AZ::Component overrides.
        //! @{
        void Activate() override;
        void Deactivate() override;
        //! }@

        //! IFilterEntityManager overrides.
        //! @{
        bool IsEntityFiltered(AZ::Entity* entity, Multiplayer::ConstNetworkEntityHandle controllerEntity, AzNetworking::ConnectionId connectionId) override;
        //! }@

    private:
        const AZ::Name m_occlusionViewName{ "ExampleFilteredEntityView" };
    };
}
