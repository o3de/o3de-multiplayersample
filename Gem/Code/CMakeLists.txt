#
# Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
#
# SPDX-License-Identifier: Apache-2.0 OR MIT
#
#

o3de_pal_dir(pal_dir ${CMAKE_CURRENT_LIST_DIR}/Platform/${PAL_PLATFORM_NAME} "${gem_restricted_path}" "${gem_path}" "${gem_parent_relative_path}")

ly_add_target(
    NAME MultiplayerSample.Client.Static STATIC
    NAMESPACE Gem
    FILES_CMAKE
        multiplayersample_autogen_files.cmake
        multiplayersample_files.cmake
        multiplayersample_client_files.cmake
        ${pal_dir}/multiplayersample_${PAL_PLATFORM_NAME_LOWERCASE}_files.cmake
    INCLUDE_DIRECTORIES
        PRIVATE
            Source
            .
        PUBLIC
            Include
    BUILD_DEPENDENCIES
        PUBLIC
            Gem::DebugDraw
            Gem::LyShine
            Gem::StartingPointInput
            Gem::EMotionFXStaticLib
            Gem::PhysX
            Gem::PopcornFX.Static
            Gem::Multiplayer.Client
            Gem::AudioSystem.API
        PRIVATE
            Gem::LmbrCentral.Static
            Gem::Multiplayer.Client.Static
            Gem::PhysX.Static
            Gem::DebugDraw.Static
            Gem::ImGui.Static
            Gem::LyShine.Static
            Gem::GameState.Static
            Gem::AWSCore.Static
            Gem::AWSGameLift.Client.Static
    AUTOGEN_RULES
        *.AutoComponent.xml,AutoComponent_Header.jinja,$path/$fileprefix.AutoComponent.h
        *.AutoComponent.xml,AutoComponent_Source.jinja,$path/$fileprefix.AutoComponent.cpp
        *.AutoComponent.xml,AutoComponentTypes_Header.jinja,$path/AutoComponentTypes.h
        *.AutoComponent.xml,AutoComponentTypes_Source.jinja,$path/AutoComponentTypes.cpp
)

ly_add_target(
    NAME MultiplayerSample.Server.Static STATIC
    NAMESPACE Gem
    FILES_CMAKE
        multiplayersample_autogen_files.cmake
        multiplayersample_files.cmake
        multiplayersample_gamelift_server_files.cmake
        ${pal_dir}/multiplayersample_${PAL_PLATFORM_NAME_LOWERCASE}_files.cmake
    INCLUDE_DIRECTORIES
        PRIVATE
            Source
            .
        PUBLIC
            Include
    BUILD_DEPENDENCIES
        PUBLIC
            Gem::StartingPointInput
            Gem::EMotionFXStaticLib
            Gem::PhysX
            Gem::Multiplayer.Server
        PRIVATE
            Gem::LmbrCentral.Static
            Gem::Multiplayer.Server.Static
            Gem::PhysX.Static
            Gem::GameState.Static
            Gem::AWSGameLift.Server.Static
    AUTOGEN_RULES
        *.AutoComponent.xml,AutoComponent_Header.jinja,$path/$fileprefix.AutoComponent.h
        *.AutoComponent.xml,AutoComponent_Source.jinja,$path/$fileprefix.AutoComponent.cpp
        *.AutoComponent.xml,AutoComponentTypes_Header.jinja,$path/AutoComponentTypes.h
        *.AutoComponent.xml,AutoComponentTypes_Source.jinja,$path/AutoComponentTypes.cpp
)

ly_add_target(
    NAME MultiplayerSample.Unified.Static STATIC
    NAMESPACE Gem
    FILES_CMAKE
        multiplayersample_autogen_files.cmake
        multiplayersample_files.cmake
        multiplayersample_client_files.cmake
        ${pal_dir}/multiplayersample_${PAL_PLATFORM_NAME_LOWERCASE}_files.cmake
    INCLUDE_DIRECTORIES
        PRIVATE
            Source
            .
        PUBLIC
            Include
    BUILD_DEPENDENCIES
        PUBLIC
            Gem::DebugDraw
            Gem::LyShine
            Gem::StartingPointInput
            Gem::EMotionFXStaticLib
            Gem::PhysX
            Gem::PopcornFX.Static
            Gem::Multiplayer
        PRIVATE
            Gem::LmbrCentral.Static
            Gem::Multiplayer.Unified.Static
            Gem::PhysX.Static
            Gem::DebugDraw.Static
            Gem::ImGui.Static
            Gem::LyShine.Static
            Gem::GameState.Static
            Gem::AWSCore.Static
            Gem::AWSGameLift.Client.Static
            Gem::AWSGameLift.Server.Static
    AUTOGEN_RULES
        *.AutoComponent.xml,AutoComponent_Header.jinja,$path/$fileprefix.AutoComponent.h
        *.AutoComponent.xml,AutoComponent_Source.jinja,$path/$fileprefix.AutoComponent.cpp
        *.AutoComponent.xml,AutoComponentTypes_Header.jinja,$path/AutoComponentTypes.h
        *.AutoComponent.xml,AutoComponentTypes_Source.jinja,$path/AutoComponentTypes.cpp
)

ly_add_target(
    NAME MultiplayerSample.Client ${PAL_TRAIT_MONOLITHIC_DRIVEN_MODULE_TYPE}
    NAMESPACE Gem
    FILES_CMAKE
        multiplayersample_shared_files.cmake
        ../../multiplayersample_asset_files.cmake
    INCLUDE_DIRECTORIES
        PRIVATE
            Source
            .
        PUBLIC
            Include
    BUILD_DEPENDENCIES
        PRIVATE
            Gem::Multiplayer.Client.Static
            Gem::MultiplayerSample.Client.Static
            Gem::Atom_AtomBridge.Static
)

ly_add_target(
    NAME MultiplayerSample.Server ${PAL_TRAIT_MONOLITHIC_DRIVEN_MODULE_TYPE}
    NAMESPACE Gem
    FILES_CMAKE
        multiplayersample_shared_files.cmake
        ../../multiplayersample_asset_files.cmake
    INCLUDE_DIRECTORIES
        PRIVATE
            Source
            .
        PUBLIC
            Include
    BUILD_DEPENDENCIES
        PRIVATE
            Gem::Multiplayer.Server.Static
            Gem::MultiplayerSample.Server.Static
            Gem::Atom_AtomBridge.Static
)

ly_add_target(
    NAME MultiplayerSample ${PAL_TRAIT_MONOLITHIC_DRIVEN_MODULE_TYPE}
    NAMESPACE Gem
    FILES_CMAKE
        multiplayersample_shared_files.cmake
        ../../multiplayersample_asset_files.cmake
    INCLUDE_DIRECTORIES
        PRIVATE
            Source
            .
        PUBLIC
            Include
    BUILD_DEPENDENCIES
        PRIVATE
            Gem::Multiplayer.Unified.Static
            Gem::MultiplayerSample.Unified.Static
            Gem::Atom_AtomBridge.Static
)

# if enabled, MultiplayerSample is used by all kinds of applications
ly_create_alias(NAME MultiplayerSample.Builders NAMESPACE Gem TARGETS Gem::MultiplayerSample)
ly_create_alias(NAME MultiplayerSample.Tools    NAMESPACE Gem TARGETS Gem::MultiplayerSample)
ly_create_alias(NAME MultiplayerSample.Clients  NAMESPACE Gem TARGETS Gem::MultiplayerSample.Client)
ly_create_alias(NAME MultiplayerSample.Servers  NAMESPACE Gem TARGETS Gem::MultiplayerSample.Server)
ly_create_alias(NAME MultiplayerSample.Unified  NAMESPACE Gem TARGETS Gem::MultiplayerSample)

################################################################################
# Gem dependencies
################################################################################

# Maps the MultiplayerSample Project with the specified list of enabled gems
ly_enable_gems(PROJECT_NAME MultiplayerSample GEM_FILE enabled_gems.cmake)

# If we build a server, then add the project name to the list of server launcher projects
if(PAL_TRAIT_BUILD_SERVER_SUPPORTED)
    set_property(GLOBAL APPEND PROPERTY LY_LAUNCHER_SERVER_PROJECTS MultiplayerSample)
endif()

# If we build a server, then add the project name to the list of server launcher projects
if(PAL_TRAIT_BUILD_UNIFIED_SUPPORTED)
    set_property(GLOBAL APPEND PROPERTY LY_LAUNCHER_UNIFIED_PROJECTS MultiplayerSample)
endif()

set_property(TARGET MultiplayerSample APPEND PROPERTY GAMELAUNCHER_ADDITIONAL_VS_DEBUGGER_COMMAND_ARGUMENTS "--console-command-file=\"launch_client.cfg\"")
set_property(TARGET MultiplayerSample APPEND PROPERTY SERVERLAUNCHER_ADDITIONAL_VS_DEBUGGER_COMMAND_ARGUMENTS "--console-command-file=\"launch_server.cfg\"")
set_property(TARGET MultiplayerSample APPEND PROPERTY UNIFIEDLAUNCHER_ADDITIONAL_VS_DEBUGGER_COMMAND_ARGUMENTS "--console-command-file=\"launch_server.cfg\"")