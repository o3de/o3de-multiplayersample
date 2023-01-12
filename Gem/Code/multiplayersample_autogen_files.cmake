#
# Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
# 
# SPDX-License-Identifier: Apache-2.0 OR MIT
#
#

set(FILES
    ${LY_ROOT_FOLDER}/Gems/Multiplayer/Code/Include/Multiplayer/AutoGen/AutoComponent_Common.jinja
    ${LY_ROOT_FOLDER}/Gems/Multiplayer/Code/Include/Multiplayer/AutoGen/AutoComponent_Header.jinja
    ${LY_ROOT_FOLDER}/Gems/Multiplayer/Code/Include/Multiplayer/AutoGen/AutoComponent_Source.jinja
    ${LY_ROOT_FOLDER}/Gems/Multiplayer/Code/Include/Multiplayer/AutoGen/AutoComponentTypes_Header.jinja
    ${LY_ROOT_FOLDER}/Gems/Multiplayer/Code/Include/Multiplayer/AutoGen/AutoComponentTypes_Source.jinja
    
    # Scripting samples
    Source/AutoGen/ScriptingPlayerMovementComponent.AutoComponent.xml
    
    Source/AutoGen/NetworkAiComponent.AutoComponent.xml
    Source/AutoGen/NetworkAnimationComponent.AutoComponent.xml
    Source/AutoGen/NetworkHealthComponent.AutoComponent.xml
    Source/AutoGen/NetworkPlayerSpawnerComponent.AutoComponent.xml
    Source/AutoGen/NetworkRandomComponent.AutoComponent.xml
    Source/AutoGen/NetworkWeaponsComponent.AutoComponent.xml
    Source/AutoGen/NetworkSimplePlayerCameraComponent.AutoComponent.xml
    Source/AutoGen/NetworkStressTestComponent.AutoComponent.xml
    Source/AutoGen/NetworkPlayerMovementComponent.AutoComponent.xml
    Source/AutoGen/NetworkTestSpawnerComponent.AutoComponent.xml
    Source/AutoGen/NetworkRandomImpulseComponent.AutoComponent.xml
    Source/AutoGen/NetworkRandomTranslateComponent.AutoComponent.xml
)
