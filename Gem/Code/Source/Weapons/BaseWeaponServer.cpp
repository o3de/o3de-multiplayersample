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



#include "stdafx.h"
#include "Weapons/BaseWeaponServer.h"
#include "Weapons/ShapeCastWeaponServer.h"
#include "Weapons/ProjectileWeaponServer.h"
#include "Physics/NetworkPhysicalWorldQueries.h"


using namespace NovaNet;


BaseWeaponServer::BaseWeaponServer(const BaseWeapon::ConstructParams& a_ConstructParams, BaseWeaponServerListener& a_Listener)
:   BaseWeapon(a_ConstructParams)
,   m_Listener(a_Listener)
{
    ;
}


int32_t BaseWeaponServer::GetAmmoTypeSurfaceIndex() const
{
    return -1;
}


void BaseWeaponServer::DispatchHitEvents(const IntersectResults& a_GatherResults, const ActivateEvent& a_EventData)
{
    const Quat rotation = Quat::CreateLookRotation((a_EventData.GetTargetPosition() - a_EventData.GetInitialTransform().m_Position).Normalized());

    HitEvent hitEvent(Transform(a_EventData.GetTargetPosition(), rotation), a_EventData.GetShooterId(), INVALID_ENTITY_ID, HitEntities());

    for (auto gatherResult : a_GatherResults.m_Intersections)
    {
        hitEvent.ModifyHitEntities().PushBack(HitEntity(gatherResult.m_Position, gatherResult.m_EntityId));
    }

    // Shape-cast weapons start their gathers at the point of origin, so we pass in the initial transform position here (this does not handle projectiles)
    ServerHitInfo hitInfo(*this, a_EventData.GetInitialTransform().m_Position, hitEvent);
    m_Listener.OnHit(hitInfo);
}


ServerActivationInfo::ServerActivationInfo(const IWeapon& a_Weapon, const ActivateEvent& a_ActivateEvent)
:   m_Weapon(a_Weapon)
,   m_ActivateEvent(a_ActivateEvent)
{
    ;
}


ServerHitInfo::ServerHitInfo(const IWeapon& a_Weapon, const Vec3& a_GatherOrigin, const HitEvent& a_HitEvent)
:   m_Weapon(a_Weapon)
,   m_GatherOrigin(a_GatherOrigin)
,   m_HitEvent(a_HitEvent)
{
    ;
}


BaseWeaponServer* NovaNet::Create(const BaseWeapon::ConstructParams& a_ConstructionParams, BaseWeaponServerListener& a_Listener)
{
    switch (a_ConstructionParams.m_Params.GetWeaponType())
    {
    case EWeaponTypes::ShapeCast:
        return new ShapeCastWeaponServer(a_ConstructionParams, a_Listener);

    case EWeaponTypes::Projectile:
        return new ProjectileWeaponServer(a_ConstructionParams, a_Listener);

    case EWeaponTypes::None:
    default:
        break;
    }

    return nullptr;
}
