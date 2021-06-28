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
    //! @class ExampleFilteredEntityComponent
    //! @brief An example of using IFilterEntityManager to filter entities to clients.
    class ExampleFilteredEntityComponent final
        : public AZ::Component
        , public Multiplayer::IFilterEntityManager
    {
    public:
        AZ_COMPONENT(MultiplayerSample::ExampleFilteredEntityComponent, "{7BF3BF1D-383A-40E7-BCF2-1ED5B2D2A43C}");

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
        bool m_enabled = true;
    };
}
