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

#include "Weapons/BaseWeaponClient.h"


namespace NovaNet
{
    /**
     * @class ShapeCastWeaponClient
     * @brief Client-side weapon class for all instant hit area and trace weapons
     */
    class ShapeCastWeaponClient AZ_FINAL
    :   public BaseWeaponClient
    {
    public:

        /**
         * Constructor
         * 
         * @param a_ConstructParams required construction parameters for the weapon instance
         * @param a_Listener        listener for BaseWeaponClient event callbacks
         */
        ShapeCastWeaponClient(const BaseWeapon::ConstructParams& a_ConstructParams, BaseWeaponClientListener& a_Listener);

        /**
         * Destructor
         */
        virtual ~ShapeCastWeaponClient();

        /**
         * BaseWeaponClient interface
         * Internally called on every weapon activation
         * 
         * @param a_DeltaTime   the time in seconds this activate event corresponds to
         * @param a_WeaponState the weapons internal state structure
         * @param a_Predicted   if true, the weapon activation is predictive
         * @param a_Replay      if true, these activations are happening during replay due to a correction
         * @param a_EventData   contains details of the activation event
         */
        virtual void ActivateClient(float a_DeltaTime, WeaponState& a_WeaponState, bool a_Predicted, bool a_Replay, ActivateEvent& a_EventData) AZ_OVERRIDE;

    private:

        /**
         * BaseWeapon interface
         * Ticks the active shots for this weapon
         * 
         * @param a_WeaponState reference to the predictive state for this weapon
         * @param a_DeltaTime   the amount of time we are ticking over
         */
        virtual void TickActiveShots(WeaponState& a_WeaponState, float a_DeltaTime) AZ_OVERRIDE;

        // Do not allow assignment
        ShapeCastWeaponClient& operator =(const ShapeCastWeaponClient&) = delete;

    };
}
