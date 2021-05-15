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
            AZ::AzNetworking
            Gem::Multiplayer
            Gem::StartingPointInput
            .
        PUBLIC
            Include
            ${common_dir}
    BUILD_DEPENDENCIES
        PUBLIC
            AZ::AzGameFramework
            AZ::AzNetworking
            Gem::Multiplayer
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
    OUTPUT_NAME Gem.MultiplayerSample.8d7f210742af4b8c8813d593f4c46cf2.v0.1.0
    FILES_CMAKE
        multiplayersample_shared_files.cmake
        ../../multiplayersample_asset_files.cmake
    INCLUDE_DIRECTORIES
        PRIVATE
            Source
            .
    BUILD_DEPENDENCIES
        PRIVATE
            Gem::Atom_AtomBridge.Static
            Gem::MultiplayerSample.Static
            Gem::LmbrCentral.Static
)

################################################################################
# Gem dependencies
################################################################################
ly_add_project_dependencies(
    PROJECT_NAME
        MultiplayerSample
    TARGETS 
        MultiplayerSample.GameLauncher
    DEPENDENCIES_FILES 
        runtime_dependencies.cmake
        ${pal_dir}/runtime_dependencies.cmake
)

if(PAL_TRAIT_BUILD_HOST_TOOLS)
    ly_add_project_dependencies(
        PROJECT_NAME
            MultiplayerSample
        TARGETS
            AssetBuilder
            AssetProcessor
            AssetProcessorBatch
            Editor
        DEPENDENCIES_FILES
            tool_dependencies.cmake
            ${pal_dir}/tool_dependencies.cmake
    )
endif()

if(PAL_TRAIT_BUILD_SERVER_SUPPORTED)
    ly_add_target(
        NAME MultiplayerSample.Server ${PAL_TRAIT_MONOLITHIC_DRIVEN_MODULE_TYPE}
        NAMESPACE Gem
        OUTPUT_NAME Gem.MultiplayerSample.Server.8d7f210742af4b8c8813d593f4c46cf2.v0.1.0
        FILES_CMAKE
            multiplayersample_shared_files.cmake
        INCLUDE_DIRECTORIES
            PRIVATE
                Source
                .
        BUILD_DEPENDENCIES
            PRIVATE
                Gem::MultiplayerSample.Static
    )

    ly_add_project_dependencies(
        PROJECT_NAME
            MultiplayerSample
        TARGETS
            MultiplayerSample.ServerLauncher
        DEPENDENCIES_FILES
            runtime_dependencies.cmake
    )
    set_property(GLOBAL APPEND PROPERTY LY_LAUNCHER_SERVER_PROJECTS MultiplayerSample)

endif()
