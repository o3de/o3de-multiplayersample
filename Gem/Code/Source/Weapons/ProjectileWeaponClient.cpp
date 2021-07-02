/*
* All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
* its licensors.
*
* For complete copyright and license terms please see the LICENSE at the root of this
* distribution (the "License"). All use of this software is governed by the License,
* or, if provided, by the license below or the license accompanying this file. Do not
* remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*
*/



#include "StdAfx.h"
#include "Weapons/ProjectileWeaponClient.h"
#include "Component/NovaNet/ProjectileComponentClient.h"
#include "Network/INovaGame.h"
#include "Component/NovaNet/IDebugDrawComponent.h"
#include "Environment/ConsoleVar.h"
#include <unordered_map>
#include "GameTime/GameTimeManager.h"


using namespace NovaNet;


ProjectileWeaponClient::ProjectileWeaponClient(const BaseWeapon::ConstructParams& a_ConstructParams, BaseWeaponClientListener& a_Listener)
:   BaseWeaponClient(a_ConstructParams, a_Listener)
{
    ;
}


ProjectileWeaponClient::~ProjectileWeaponClient()
{
    ;
}


void ProjectileWeaponClient::ActivateClient(float a_DeltaTime, WeaponState& a_WeaponState, bool a_Predicted, bool a_Replay, ActivateEvent& a_EventData)
{
    const bool bValidate = a_Predicted; // Validate only on predicted activations, since simulated guys activate off the server authority

    if (ActivateInternal(a_WeaponState, bValidate, a_EventData))
    {
        if (a_Replay)
        {
            // Skip out on all the remaining activation logic if this is a replay..  We do not want to spawn particles or sounds or other effects during a replay event
            return;
        }

        m_Listener.OnActivate(ClientActivationInfo(*this, a_EventData));

        if (a_Predicted)
        {
            const Transform transform = a_EventData.GetInitialTransform();

            // Create this entity at the unaltered time since it will update itself outside of process input
            EmptyEntityIterator iterator;
            ScopedGameTime scopedTime(gNovaGame->GetGameTimeManager().GetUnalteredGameTime(), nullptr, iterator);
            EntityPtr createdEntity = gNovaGame->GetEntityManager().CreateSingleEntityImmediate(SliceEntryId(m_Params.GetProjectileAsset()), AutoActivate::Activate, transform);

            if (ProjectileComponent::Autonomous *pProjectileComponent = FindController<ProjectileComponent::Autonomous>(createdEntity))
            {
                const ProjectileComponent::Predictable::ConstructParams constructParams(a_EventData.GetShooterId(), GetWeaponIndex(), m_Params.GetGatherParams(), 0.0f);
                pProjectileComponent->PostInit(constructParams, transform);
            }
        }
    }
}


void ProjectileWeaponClient::TickActiveShots(WeaponState& a_WeaponState, float a_DeltaTime)
{
    ; // no-op, projectiles spawn as individual entities that tick themselves..  if a game does client steered projectiles then this pattern may need to change
}
