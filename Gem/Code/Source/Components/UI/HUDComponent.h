/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/Component/Component.h>
#include <AzCore/EBus/Event.h>
#include <LyShine/Bus/World/UiCanvasRefBus.h>

namespace MultiplayerSample
{
    class HUDComponent
        : public AZ::Component
        , UiCanvasAssetRefNotificationBus::Handler
    {
    public:
        AZ_COMPONENT(MultiplayerSample::HUDComponent, "{8061E5D2-A1F7-4B40-9AAC-8FF14BD094FC}");

        /*
        * Reflects component data into the reflection contexts, including the serialization, edit, and behavior contexts.
        */
        static void Reflect(AZ::ReflectContext* context);
        
        /*
        * Specifies the services that this component requires.
        * The system activates the required services before it activates this component.
        * It also deactivates the required services after it deactivates this component.
        * If a required service is missing before this component is activated, the system
        * returns an error and does not activate this component.
        */
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
    
    protected:
        void Activate() override;
        void Deactivate() override;

    private:
        // UiCanvasAssetRefNotificationBus overrides ...
        void OnCanvasLoadedIntoEntity(AZ::EntityId uiCanvasEntity) override;

        void SetRoundNumberText(uint16_t round);
        void SetRoundTimerText(float time);
    
    private:
        AZ::EntityId m_uiCanvasId;
        int m_roundNumberId = 0;
        int m_roundTimerId = 0;
        AZStd::string m_roundNumberText = "";
        AZStd::string m_roundTimerText = "";
        AZ::EventHandler<uint16_t> m_roundNumberHandler; 
        AZ::EventHandler<float> m_roundTimerHandler;
    };
} // namespace MultiplayerSample
