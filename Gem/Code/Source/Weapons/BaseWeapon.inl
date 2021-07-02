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


#pragma once


namespace NovaNet
{
    inline WeaponIndex BaseWeapon::GetWeaponIndex() const
    {
        return m_WeaponIndex;
    }


    inline const WeaponParams& BaseWeapon::GetParams() const
    {
        return m_Params;
    }


    inline const FireParams& BaseWeapon::GetFireParams() const
    {
        return m_FireParams;
    }


    inline void BaseWeapon::SetFireParams(const FireParams& a_Params)
    {
        m_FireParams = a_Params;
    }


    inline const ClientEffect& BaseWeapon::GetActivateEffect() const
    {
        return m_ActivateEffect;
    }


    inline const ClientEffect &BaseWeapon::GetImpactEffect() const
    {
        return m_ImpactEffect;
    }


    inline const ClientEffect& BaseWeapon::GetDamageEffect() const
    {
        return m_DamageEffect;
    }
}
