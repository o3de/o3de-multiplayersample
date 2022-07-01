/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/Component/Component.h>
#include <LyShine/Bus/World/UiCanvasRefBus.h>

namespace MultiplayerSample
{
    class UiCanvasDemoPlacardComponent
        : public AZ::Component
        , UiCanvasAssetRefNotificationBus::Handler
    {
    public:
        AZ_COMPONENT(MultiplayerSample::UiCanvasDemoPlacardComponent, "{8ED1F410-04CA-4180-BF2F-D24A1BB4BF7D}");

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

        void SetUiCanvasText(AZ::EntityId uiCanvasEntityId);
    
    private:
        int m_placardTextboxUiElementId = 0;
        AZStd::string m_placardText = "Default Placard Text";
    };
} // namespace MultiplayerSample
