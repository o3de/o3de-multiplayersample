/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <Source/Components/NetworkRandomComponent.h>

namespace MultiplayerSample
{
    void NetworkRandomComponent::NetworkRandomComponent::Reflect(AZ::ReflectContext* context)
    {
        AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context);
        if (serializeContext)
        {
            serializeContext->Class<NetworkRandomComponent, NetworkRandomComponentBase>()
                ->Version(1);
        }
        NetworkRandomComponentBase::Reflect(context);
    }

    NetworkRandomComponent::NetworkRandomComponent()
        : m_seedEventHandler([this](const uint64_t& seed) { OnSeedChangedEvent(seed); })
    {
        if (IsNetEntityRoleAuthority())
        {
            // Setup seed on authority for proxies to pull
            AZ::BetterPseudoRandom seedGenerator;
            uint64_t seed = 0;
            seedGenerator.GetRandom(seed);
            static_cast<NetworkRandomComponentController*>(GetController())->SetSeed(seed);
            m_simpleRng.SetSeed(seed);
            m_seedInitialized = true;
        };
    }

    void NetworkRandomComponent::OnInit()
    {
        
    }

    void NetworkRandomComponent::OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        SeedAddEvent(m_seedEventHandler);
    }

    void NetworkRandomComponent::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        ;
    }

    void NetworkRandomComponent::RollRandom()
    {
        Multiplayer::INetworkTime* networkTime = Multiplayer::GetNetworkTime();
        // We should not need to roll rands in the past, history should be relied upon instead
        if (!networkTime->IsTimeRewound())
        {
            AZ_Assert(m_seedInitialized, "RNG Seed not initialized");
            int rand = m_simpleRng.GetRandom();
            static_cast<NetworkRandomComponentController*>(GetController())->SetRandom(rand);
        }
    }

    void NetworkRandomComponent::OnSeedChangedEvent([[maybe_unused]] const uint64_t& seed)
    {
        // Proxy hook to set rng seed
        static_cast<NetworkRandomComponentController*>(GetController())->SetSeed(seed);
        m_simpleRng.SetSeed(seed);
        m_seedInitialized = true;
    }

    NetworkRandomComponentController::NetworkRandomComponentController(NetworkRandomComponent& parent)
        : NetworkRandomComponentControllerBase(parent)
    {
        ;
    }

    void NetworkRandomComponentController::OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        ;
    }

    void NetworkRandomComponentController::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        ;
    }
}
