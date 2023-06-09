/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 * 
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/Component/Component.h>
#include <AzCore/Console/IConsole.h>

namespace MPSGameLift
{
    class MPSGameLiftClientSystemComponent
        : public AZ::Component
    {
    public:
        AZ_COMPONENT(MPSGameLiftClientSystemComponent, "{939D9813-2DCA-4625-B4E1-E63A6A652A26}");

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);

    protected:
        // AZ::Component interface implementation
        void Init() override;
        void Activate() override;
        void Deactivate() override;

        // Join a GameLift game session.
        // Internally, GameLift will use the game session id to generate new player session id and pass it back to this client as a ticket for connecting to the host server on GameLift.
        void JoinSession(const AZ::ConsoleCommandContainer& consoleFunctionParameters);
        AZ_CONSOLEFUNC(MPSGameLiftClientSystemComponent, JoinSession, AZ::ConsoleFunctorFlags::DontReplicate, "Join an existing game session");

    private:
        void JoinSessionInternal(AZStd::string_view gameSessionId, const AZ::Uuid& playerId);

        AZ::Uuid m_playerId = AZ::Uuid::Create(); // Unique identifier for the current player inside the game session
    };
}
