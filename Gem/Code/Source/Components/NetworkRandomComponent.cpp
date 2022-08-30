/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AzCore/Math/Random.h>
#include <Source/Components/NetworkRandomComponent.h>

namespace MultiplayerSample
{
    uint64_t NetworkRandomComponentController::GetRandomUint64()
    {
        // Reimplements SimpleLcgRandom's rand int with a synchronized seed
        uint64_t& seed = ModifySeed();
        seed = (GetSeed() * 0x5DEECE66DLL + 0xBLL) & ((1LL << 48) - 1);
        return seed;
    }

    int NetworkRandomComponentController::GetRandomInt()
    {
        // Reimplements SimpleLcgRandom's rand int with a synchronized seed
        return static_cast<unsigned int>(GetRandomUint64() >> 16);
    }

    float NetworkRandomComponentController::GetRandomFloat()
    {
        // Reimplements SimpleLcgRandom's rand float with a synchronized seed
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

    NetworkRandomComponentController::NetworkRandomComponentController(NetworkRandomComponent& parent)
        : NetworkRandomComponentControllerBase(parent)
    {
        if (IsNetEntityRoleAuthority())
        {
            // Setup seed on authority for proxies to pull
            AZ::BetterPseudoRandom seedGenerator;
            uint64_t seed = 0;
            seedGenerator.GetRandom(seed);
            SetSeed(seed);
        };
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
