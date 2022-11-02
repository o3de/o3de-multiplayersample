/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AtomLyIntegration/CommonFeatures/Material/MaterialComponentBus.h>
#include <DebugDraw/DebugDrawBus.h>
#include <Source/Components/RpcTesterComponent.h>

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
        AZStd::string testType;
        if (GetTestAutoToAuthorityRPC())
        {
            testType = "Auto->Auth";
        }
        else if (GetTestServerToAuthorityRPC())
        {
            testType = "Server->Auth";
        }
        else if (GetTestAuthorityToAutoRPC())
        {
            testType = "Auth->Auto";
        }
        else if (GetTestAuthorityToClientRPC())
        {
            testType = "Auth->Client";
        }

        DebugDraw::DebugDrawRequestBus::Broadcast(&DebugDraw::DebugDrawRequestBus::Events::DrawTextOnEntity,
            GetEntityId(), testType, AZ::Colors::White, -1.f);

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
        if (GetController())
        {
            auto controller = static_cast<RpcTesterComponentController*>(GetController());
            if (GetTestAutoToAuthorityRPC())
            {
                controller->RPC_AutonomousToAuthority();
            }
            else if (GetTestServerToAuthorityRPC())
            {
                RPC_ServerToAuthority();
            }
            else if (GetTestAuthorityToAutoRPC())
            {
                controller->RPC_AuthorityToAutonomous();
            }
            else if (GetTestAuthorityToClientRPC())
            {
                controller->RPC_TestPassed();
            }
        }
    }


    RpcTesterComponentController::RpcTesterComponentController(RpcTesterComponent& parent)
        : RpcTesterComponentControllerBase(parent)
    {
    }

    void RpcTesterComponentController::OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        GetParent().m_delayTestRun.Enqueue(AZ::TimeMs{ 2000 }, false);
    }

    void RpcTesterComponentController::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
    }

    void RpcTesterComponentController::HandleRPC_AutonomousToAuthority([[maybe_unused]] AzNetworking::IConnection* invokingConnection)
    {
        RPC_TestPassed();
    }

    void RpcTesterComponentController::HandleRPC_AuthorityToAutonomous([[maybe_unused]] AzNetworking::IConnection* invokingConnection)
    {
        RPC_TestPassed();
    }

    void RpcTesterComponentController::HandleRPC_ServerToAuthority([[maybe_unused]] AzNetworking::IConnection* invokingConnection)
    {
        RPC_TestPassed();
    }
}
