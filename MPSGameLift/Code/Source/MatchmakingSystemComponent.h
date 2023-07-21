/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 * 
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/Component/Component.h>
#include <AzCore/EBus/ScheduledEvent.h>
#include <MPSGameLift/IMatchmaking.h>


namespace MPSGameLift
{
    class MatchmakingSystemComponent final
        : public AZ::Component
        , public IMatchmaking
    {
    public:
        AZ_COMPONENT(MatchmakingSystemComponent, "{BF5F9343-63B5-4703-89ED-9CDBF4FE6004}");
        static void Reflect(AZ::ReflectContext* context);
        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);

        // IMatchmaking overrides...
        bool RequestMatch(const RegionalLatencies& regionalLatencies) override;
        AZStd::string GetTicketId() const override { return m_ticketId; }

     protected:
        void Activate() override;
        void Deactivate() override;
        void RequestMatchStatus();

    private:
        AZStd::string m_ticketId;
        AZ::ScheduledEvent m_requestMatchStatusEvent = AZ::ScheduledEvent([this] { this->RequestMatchStatus(); }, AZ::Name("MPS Request Match Status"));

     };
}
