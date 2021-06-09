#
# All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
# its licensors.
#
# For complete copyright and license terms please see the LICENSE at the root of this
# distribution (the "License"). All use of this software is governed by the License,
# or, if provided, by the license below or the license accompanying this file. Do not
# remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#

ly_get_list_relative_pal_filename(pal_dir ${CMAKE_CURRENT_LIST_DIR}/Platform/${PAL_PLATFORM_NAME})

ly_add_target(
    NAME MultiplayerSample.Static STATIC
    NAMESPACE Gem
    FILES_CMAKE
        multiplayersample_files.cmake
        multiplayersample_autogen_files.cmake
        ${pal_dir}/multiplayersample_${PAL_PLATFORM_NAME_LOWERCASE}_files.cmake
    INCLUDE_DIRECTORIES
        PRIVATE
            Source
            .
        PUBLIC
            Include
    BUILD_DEPENDENCIES
        PUBLIC
            AZ::AzCore
            AZ::AzGameFramework
            AZ::AzNetworking
            Gem::Multiplayer
            Gem::EMotionFXStaticLib
            Gem::PhysX
            Gem::StartingPointInput
        PRIVATE
            Gem::LmbrCentral.Static
            Gem::Multiplayer.Static
    AUTOGEN_RULES
        *.AutoComponent.xml,AutoComponent_Header.jinja,$path/$fileprefix.AutoComponent.h
        *.AutoComponent.xml,AutoComponent_Source.jinja,$path/$fileprefix.AutoComponent.cpp
        *.AutoComponent.xml,AutoComponentTypes_Header.jinja,$path/AutoComponentTypes.h
        *.AutoComponent.xml,AutoComponentTypes_Source.jinja,$path/AutoComponentTypes.cpp
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
            Gem::MultiplayerSample.Static
            Gem::Atom_AtomBridge.Static
)

# if enabled, MultiplayerSample is used by all kinds of applications
ly_create_alias(NAME MultiplayerSample.Builders NAMESPACE Gem TARGETS Gem::MultiplayerSample)
ly_create_alias(NAME MultiplayerSample.Tools    NAMESPACE Gem TARGETS Gem::MultiplayerSample)
ly_create_alias(NAME MultiplayerSample.Clients  NAMESPACE Gem TARGETS Gem::MultiplayerSample)
ly_create_alias(NAME MultiplayerSample.Servers  NAMESPACE Gem TARGETS Gem::MultiplayerSample)

################################################################################
# Gem dependencies
################################################################################

# The GameLauncher uses "Clients" gem variants:
ly_enable_gems(PROJECT_NAME MultiplayerSample GEM_FILE enabled_gems.cmake
    TARGETS MultiplayerSample.GameLauncher
    VARIANTS Clients)

# If we build a server, then apply the gems to the server
if(PAL_TRAIT_BUILD_SERVER_SUPPORTED)
    # if we're making a server, then add the "Server" gem variants to it:
    ly_enable_gems(PROJECT_NAME MultiplayerSample GEM_FILE enabled_gems.cmake
        TARGETS MultiplayerSample.ServerLauncher
        VARIANTS Servers)
    
    set_property(GLOBAL APPEND PROPERTY LY_LAUNCHER_SERVER_PROJECTS MultiplayerSample)
endif()

if (PAL_TRAIT_BUILD_HOST_TOOLS)
    # The Editor uses "Tools" gem variants:
    ly_enable_gems(
        PROJECT_NAME MultiplayerSample GEM_FILE enabled_gems.cmake
        TARGETS Editor
        VARIANTS Tools)

    # The pipeline tools use "Builders" gem variants:
    ly_enable_gems(
        PROJECT_NAME MultiplayerSample GEM_FILE enabled_gems.cmake
        TARGETS Editor AssetBuilder AssetProcessor AssetProcessorBatch
        VARIANTS Builders)
endif()