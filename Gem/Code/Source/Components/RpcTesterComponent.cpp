/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AtomLyIntegration/CommonFeatures/Material/MaterialComponentBus.h>
#include <Source/Components/RpcTesterComponent.h>

#if AZ_TRAIT_CLIENT
#include <DebugDraw/DebugDrawBus.h>
#endif

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

#if AZ_TRAIT_CLIENT
        DebugDraw::DebugDrawRequestBus::Broadcast(&DebugDraw::DebugDrawRequestBus::Events::DrawTextOnEntity,
            GetEntityId(), testType, AZ::Colors::White, -1.f);
#endif

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

#if AZ_TRAIT_CLIENT
    void RpcTesterComponent::HandleRPC_TestPassed([[maybe_unused]] AzNetworking::IConnection* invokingConnection)
    {
        if (AZ::Render::MaterialComponentRequests* material =
            AZ::Render::MaterialComponentRequestBus::FindFirstHandler(GetEntityId()))
        {
            material->SetMaterialAssetIdOnDefaultSlot(GetTestPassedMaterial().GetId());
        }
    }
#endif

    void RpcTesterComponent::RunTests()
    {
        if (GetController())
        {
            auto controller = static_cast<RpcTesterComponentController*>(GetController());
            
#if AZ_TRAIT_CLIENT
            if (GetTestAutoToAuthorityRPC())
            {
                controller->RPC_AutonomousToAuthority();
                return;
            }
#endif

#if AZ_TRAIT_SERVER
            if (GetTestServerToAuthorityRPC())
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
#endif
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

#if AZ_TRAIT_CLIENT
    void RpcTesterComponentController::HandleRPC_AuthorityToAutonomous([[maybe_unused]] AzNetworking::IConnection* invokingConnection)
    {
        ;
    }
#endif

#if AZ_TRAIT_SERVER
    void RpcTesterComponentController::HandleRPC_AutonomousToAuthority([[maybe_unused]] AzNetworking::IConnection* invokingConnection)
    {
        RPC_TestPassed();
    }

    void RpcTesterComponentController::HandleRPC_ServerToAuthority([[maybe_unused]] AzNetworking::IConnection* invokingConnection)
    {
        RPC_TestPassed();
    }
#endif
}
