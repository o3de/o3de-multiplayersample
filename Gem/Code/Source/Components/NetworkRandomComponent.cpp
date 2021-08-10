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

    uint64_t NetworkRandomComponent::GetRandomUint64()
    {
        AZ_Assert(m_seedInitialized, "RNG Seed not initialized");
        uint64_t seed = m_simpleRng.Getu64Random();
        static_cast<NetworkRandomComponentController*>(GetController())->SetSeed(seed);
        return seed;
    }

    int NetworkRandomComponent::GetRandomInt()
    {
        // Reimplements SimpleLcgRandom's rand int with a synchronized seed
        AZ_Assert(m_seedInitialized, "RNG Seed not initialized");
        return static_cast<unsigned int>(GetRandomUint64() >> 16);
    }

    float NetworkRandomComponent::GetRandomFloat()
    {
        // Reimplements SimpleLcgRandom's rand float with a synchronized seed
        AZ_Assert(m_seedInitialized, "RNG Seed not initialized");
        unsigned int r = GetRandomInt();
            r &= 0x007fffff; //sets mantissa to random bits
            r |= 0x3f800000; //result is in [1,2), uniformly distributed
            union
            {
                float f;
                unsigned int i;
            } u;
            u.i = r;
            return u.f - 1.0f;
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
