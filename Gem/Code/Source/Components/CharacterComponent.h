/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 * 
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <Source/AutoGen/CharacterComponent.AutoComponent.h>
#include <Multiplayer/Components/NetBindComponent.h>
#include <PhysX/CharacterGameplayBus.h>

namespace Physics
{
    class Character;
}

namespace MultiplayerSample
{
    class CharacterComponent
        : public CharacterComponentBase
        , private PhysX::CharacterGameplayRequestBus::Handler
    {
        friend class CharacterComponentController;

    public:
        AZ_MULTIPLAYER_COMPONENT(MultiplayerSample::CharacterComponent, s_characterComponentConcreteUuid, MultiplayerSample::CharacterComponentBase);

        static void Reflect(AZ::ReflectContext* context);

        CharacterComponent();

        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
        {
            incompatible.push_back(AZ_CRC_CE("NetworkRigidBodyService"));
        }

        void OnInit() override;
        void OnActivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
        void OnDeactivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;

    private:
        void OnTranslationChangedEvent(const AZ::Vector3& translation);
        void OnSyncRewind();

        // CharacterGameplayRequestBus
        bool IsOnGround() const override;
        float GetGravityMultiplier() const override { return {}; }
        void SetGravityMultiplier([[maybe_unused]] float gravityMultiplier) override {}
        AZ::Vector3 GetFallingVelocity() const override { return {}; }
        void SetFallingVelocity([[maybe_unused]] const AZ::Vector3& fallingVelocity) override {}

        Physics::Character* m_physicsCharacter = nullptr;
        Multiplayer::EntitySyncRewindEvent::Handler m_syncRewindHandler = Multiplayer::EntitySyncRewindEvent::Handler([this]() { OnSyncRewind(); });
        AZ::Event<AZ::Vector3>::Handler m_translationEventHandler;
    };

    class CharacterComponentController
        : public CharacterComponentControllerBase
    {
    public:
        CharacterComponentController(CharacterComponent& parent);

        void OnActivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
        void OnDeactivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;

        AZ::Vector3 TryMoveWithVelocity(const AZ::Vector3& velocity, float deltaTime);
    };
}
