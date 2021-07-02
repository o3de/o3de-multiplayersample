/*
 * Copyright (c) Contributors to the Open 3D Engine Project
 * 
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <Source/AutoGen/NetworkCharacterComponent.AutoComponent.h>
#include <Multiplayer/Components/NetBindComponent.h>

namespace Physics
{
    class Character;
}

namespace MultiplayerSample
{
    class NetworkCharacterComponent
        : public NetworkCharacterComponentBase
    {
        friend class NetworkCharacterComponentController;

    public:
        AZ_MULTIPLAYER_COMPONENT(MultiplayerSample::NetworkCharacterComponent, s_networkCharacterComponentConcreteUuid, MultiplayerSample::NetworkCharacterComponentBase);

        static void Reflect(AZ::ReflectContext* context);

        NetworkCharacterComponent();

        void OnInit() override;
        void OnActivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
        void OnDeactivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;

    private:
        void OnTranslationChangedEvent(const AZ::Vector3& translation);
        void OnSyncRewind();

        Physics::Character* m_physicsCharacter = nullptr;
        Multiplayer::EntitySyncRewindEvent::Handler m_syncRewindHandler = Multiplayer::EntitySyncRewindEvent::Handler([this]() { OnSyncRewind(); });
        AZ::Event<AZ::Vector3>::Handler m_translationEventHandler;
    };

    class NetworkCharacterComponentController
        : public NetworkCharacterComponentControllerBase
    {
    public:
        NetworkCharacterComponentController(NetworkCharacterComponent& parent);

        void OnActivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
        void OnDeactivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;

        AZ::Vector3 TryMoveWithVelocity(const AZ::Vector3& velocity, float deltaTime);
    };
}
