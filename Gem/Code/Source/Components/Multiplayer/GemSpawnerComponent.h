/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/Component/EntityBus.h>
#include <LmbrCentral/Scripting/TagComponentBus.h>
#include <Source/AutoGen/GemSpawnerComponent.AutoComponent.h>

namespace MultiplayerSample
{
    class GemSpawnerComponent
        : public GemSpawnerComponentBase
    {
    public:
        AZ_MULTIPLAYER_COMPONENT(MultiplayerSample::GemSpawnerComponent, s_gemSpawnerComponentConcreteUuid, MultiplayerSample::GemSpawnerComponentBase);

        static void Reflect(AZ::ReflectContext* context);

        void OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating) override {}
        void OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating) override {}
    };


    class GemSpawnerComponentController
        : public GemSpawnerComponentControllerBase
        , public AZ::EntityBus::MultiHandler
    {
    public:
        explicit GemSpawnerComponentController(GemSpawnerComponent& parent);

        void OnActivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
        void OnDeactivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;

        void SpawnGems();

        //! EntityBus
        //! @{
        void OnEntityDeactivated(const AZ::EntityId& entityId) override;
        //! @}

    private:
        void SpawnGem(const AZ::Vector3& location, const AZ::Crc32& type);
        void RemoveGems();
        
        AZStd::unordered_map<AZ::EntityId, AZStd::shared_ptr<AzFramework::EntitySpawnTicket>> m_spawnedGems;
        AZStd::unordered_map<AZ::EntityId, AZStd::shared_ptr<AzFramework::EntitySpawnTicket>> m_queuedForRemovalGems;

        //! Randomly choose a gem type from the given set of weights for the current round.
        AZ::Crc32 ChooseGemType(const LmbrCentral::Tags& tags);
    };
}
