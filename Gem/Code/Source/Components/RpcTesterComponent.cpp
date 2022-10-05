/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AtomLyIntegration/CommonFeatures/Material/MaterialComponentBus.h>
#include <DebugDraw/DebugDrawBus.h>
#include <Source/Components/RpcTesterComponent.h>

#pragma optimize("", off)

namespace MultiplayerSample
{
    void RpcTesterComponent::Reflect(AZ::ReflectContext* context)
    {
        AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context);
        if (serializeContext)
        {
            serializeContext->Class<RpcTesterComponent, RpcTesterComponentBase>()
                ->Version(1);
        }
        RpcTesterComponentBase::Reflect(context);
    }

    void RpcTesterComponent::OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        DebugDraw::DebugDrawRequestBus::Broadcast(&DebugDraw::DebugDrawRequestBus::Events::DrawTextOnEntity, 
            GetEntityId(), AZStd::string("Autonomous to Authority"), AZ::Colors::Black, -1.f);

        if (AZ::Render::MaterialComponentRequests* material =
            AZ::Render::MaterialComponentRequestBus::FindFirstHandler(GetEntityId()))
        {
            material->SetMaterialAssetIdOnDefaultSlot(GetTestWaitingMaterial().GetId());
        }
    }

    void RpcTesterComponent::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        m_delayTestRun.RemoveFromQueue();
    }

    void RpcTesterComponent::HandleRPC_TestPassed([[maybe_unused]] AzNetworking::IConnection* invokingConnection)
    {
        if (AZ::Render::MaterialComponentRequests* material =
            AZ::Render::MaterialComponentRequestBus::FindFirstHandler(GetEntityId()))
        {
            material->SetMaterialAssetIdOnDefaultSlot(GetTestPassedMaterial().GetId());
        }
    }

    void RpcTesterComponent::RunTests()
    {
        if (GetController() && IsNetEntityRoleAutonomous())
        {
            static_cast<RpcTesterComponentController*>(GetController())->RPC_AutonomousToAuthority();
        }
    }


    RpcTesterComponentController::RpcTesterComponentController(RpcTesterComponent& parent)
        : RpcTesterComponentControllerBase(parent)
    {
    }

    void RpcTesterComponentController::OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        if (IsNetEntityRoleAutonomous())
        {
            GetParent().m_delayTestRun.Enqueue(AZ::TimeMs{ 2000 }, false);
        }
    }

    void RpcTesterComponentController::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
    }

    void RpcTesterComponentController::HandleRPC_AutonomousToAuthority([[maybe_unused]] AzNetworking::IConnection* invokingConnection)
    {
        RPC_TestPassed();
    }
}

#pragma optimize("", on)
