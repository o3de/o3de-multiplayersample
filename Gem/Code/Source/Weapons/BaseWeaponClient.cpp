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
#include "Weapons/BaseWeaponClient.h"
#include "Weapons/ShapeCastWeaponClient.h"
#include "Weapons/ProjectileWeaponClient.h"
#include "Physics/NetworkPhysicalWorldQueries.h"


using namespace NovaNet;


BaseWeaponClient::BaseWeaponClient(const BaseWeapon::ConstructParams& a_ConstructParams, BaseWeaponClientListener& a_Listener)
:   BaseWeapon(a_ConstructParams)
,   m_Listener(a_Listener)
,   m_AmmoSurfaceTypeIndex(-1)
{
    const char* ammoMaterialName = a_ConstructParams.m_Params.GetAmmoMaterialType().c_str();
    ISurfaceType* pSurfaceType = gEnv->p3DEngine->GetMaterialManager()->GetSurfaceTypeManager()->GetSurfaceTypeByName(ammoMaterialName);

    if (pSurfaceType != NULL)
    {
        m_AmmoSurfaceTypeIndex = static_cast<int32_t>(pSurfaceType->GetId());
    }
}


int32_t BaseWeaponClient::GetAmmoTypeSurfaceIndex() const
{
    return m_AmmoSurfaceTypeIndex;
}


void BaseWeaponClient::DispatchHitEvents(const IntersectResults& a_GatherResults, const ActivateEvent& a_EventData, const EntityIdSet& a_PrefilteredEntityIds)
{
    HitEvent hitEvent(Transform(a_EventData.GetTargetPosition(), a_EventData.GetInitialTransform().m_Orientation), a_EventData.GetShooterId(), INVALID_ENTITY_ID, HitEntities());

    for (auto gatherResult : a_GatherResults.m_Intersections)
    {
        if (a_PrefilteredEntityIds.size() > 0)
        {
            if (a_PrefilteredEntityIds.find(gatherResult.m_EntityId) == a_PrefilteredEntityIds.end())
            {
                // Skip this hit, it was not gathered by the high-detail client physics trace, and should be filtered
                continue;
            }
        }

        hitEvent.ModifyHitEntities().PushBack(HitEntity(gatherResult.m_Position, gatherResult.m_EntityId));
    }

    ClientHitInfo hitInfo(*this, hitEvent);
    m_Listener.OnPredictHit(hitInfo);
}


ClientActivationInfo::ClientActivationInfo(const NovaNet::IWeapon& a_Weapon, const ActivateEvent& a_ActivateEvent)
:   m_Weapon(a_Weapon)
,   m_ActivateEvent(a_ActivateEvent)
{
    ;
}


ClientHitInfo::ClientHitInfo(const NovaNet::IWeapon& a_Weapon, const HitEvent& a_HitEvent)
:   m_Weapon(a_Weapon)
,   m_HitEvent(a_HitEvent)
{
    ;
}


BaseWeaponClient* NovaNet::Create(const BaseWeapon::ConstructParams& a_ConstructionParams, BaseWeaponClientListener& a_Listener)
{
    switch (a_ConstructionParams.m_Params.GetWeaponType())
    {
    case EWeaponTypes::ShapeCast:
        return new ShapeCastWeaponClient(a_ConstructionParams, a_Listener);

    case EWeaponTypes::Projectile:
        return new ProjectileWeaponClient(a_ConstructionParams, a_Listener);

    case EWeaponTypes::None:
    default:
        break;
    }

    return nullptr;
}
