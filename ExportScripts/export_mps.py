#
# Copyright (c) Contributors to the Open 3D Engine Project.
# For complete copyright and license terms please see the LICENSE at the root of https://www.github.com/o3de/o3de.
#
# SPDX-License-Identifier: Apache-2.0 OR MIT
#
#

import argparse
import logging
import sys

import o3de.export_project as exp
import o3de.enable_gem as enable_gem
import o3de.disable_gem as disable_gem
import o3de.manifest as manifest
import pathlib


def export_multiplayer_sample(ctx: exp.O3DEScriptExportContext,
                              selected_platform: str,
                              output_path: pathlib.Path,
                              should_build_tools: bool,
                              build_config: str,
                              max_bundle_size: int,
                              enable_gamelift: bool,
                              should_build_all_code: bool,
                              should_build_all_assets: bool,
                              should_build_game_launcher: bool,
                              should_build_server_launcher: bool,
                              should_build_unified_launcher: bool,
                              allow_registry_overrides: bool,
                              tools_build_path: pathlib.Path,
                              game_build_path: pathlib.Path,
                              archive_output_format: str,
                              fail_on_asset_errors: bool,
                              logger: logging.Logger):
    if not logger:
        logger = logging.getLogger()
        logger.setLevel(logging.ERROR)

    default_base_build_path = ctx.engine_path / 'build' if ctx.is_engine_centric else ctx.project_path / 'build'
    if not tools_build_path:
        tools_build_path = default_base_build_path / 'tools'
    if not game_build_path:
        game_build_path = default_base_build_path / 'game'

    seed_folder_path = ctx.project_path / 'AssetBundling' / 'SeedLists'
    seedlist_paths = [
        seed_folder_path / 'BasePopcornFxSeedList.seed',
        seed_folder_path / 'GameSeedList.seed',
        seed_folder_path / 'VFXSeedList.seed'
    ]
    if build_config == 'profile':
        # Dev branch has removed the profile seed list, but it still remains in main for now.
        # This will be removed after the next release, when both branches are synchronized
        profile_seed_list_path = seed_folder_path / 'ProfileOnlySeedList.seed'
        if profile_seed_list_path.is_file():
            seedlist_paths.extend([profile_seed_list_path])

    validated_seedslist_paths = exp.validate_project_artifact_paths(project_path=ctx.project_path,
                                                                    artifact_paths=seedlist_paths)

    exp.kill_existing_processes(ctx.project_name)

    gamelift_gem_added = False
    gamelift_gem_removed = False

    try:
        # Check if we need to enable or disable the MPSGameLift gem for the requested packages
        project_json_data = manifest.get_project_json_data(project_path=ctx.project_path)
        assert project_json_data, f"Invalid project configuration file '{ctx.project_path}/project.json'. Invalid settings."
        assert project_json_data['gem_names'], f"Invalid project configuration file '{ctx.project_path}/project.json'. Invalid settings."
        has_mps_gem = 'MPSGameLift' in project_json_data['gem_names']
        if enable_gamelift and not has_mps_gem:
            if enable_gem.enable_gem_in_project(gem_name="MPSGameLift", project_path=ctx.project_path) != 0:
                raise exp.ExportProjectError("Unable to enable the MPSGameLift gem for the project")
            gamelift_gem_added = True
        elif not enable_gamelift and has_mps_gem:
            if disable_gem.disable_gem_in_project(gem_name="MPSGameLift", project_path=ctx.project_path) != 0:
                raise exp.ExportProjectError("Unable to disable the MPSGameLift gem for the project")
            gamelift_gem_removed = True

        # Optionally build the toolchain needed to bundle the assets
        if should_build_tools:
            exp.build_export_toolchain(ctx=ctx,
                                       tools_build_path=tools_build_path,
                                       logger=logger)
        else:
            exp.validate_export_toolchain(tools_build_path=tools_build_path)

        if should_build_all_code:

            launcher_type = 0
            if should_build_game_launcher:
                launcher_type |= exp.LauncherType.GAME
            if should_build_server_launcher:
                launcher_type |= exp.LauncherType.SERVER
            if should_build_unified_launcher:
                launcher_type |= exp.LauncherType.UNIFIED

            exp.build_game_targets(ctx=ctx,
                                   build_config=build_config,
                                   game_build_path=game_build_path,
                                   launcher_types=launcher_type,
                                   allow_registry_overrides=allow_registry_overrides,
                                   logger=logger)

        if should_build_all_assets:
            exp.build_assets(ctx=ctx,
                             tools_build_path=tools_build_path,
                             fail_on_ap_errors=fail_on_asset_errors,
                             logger=logger)

            expected_bundles_path = exp.bundle_assets(ctx=ctx,
                                                      selected_platform=selected_platform,
                                                      seedlist_paths=validated_seedslist_paths,
                                                      tools_build_path=tools_build_path,
                                                      custom_asset_list_path= ctx.project_path / 'AssetBundling' / 'Bundles',
                                                      max_bundle_size=max_bundle_size)
        else:
            expected_bundles_path = ctx.project_path / 'AssetBundling' / 'Bundles'

        project_file_patterns_to_copy = []
        game_project_file_patterns_to_copy = ['launch_client.cfg']
        server_project_file_patterns_to_copy = ['launch_server.cfg']

        export_layouts = []
        if should_build_game_launcher:
            export_layouts.append(exp.ExportLayoutConfig(output_path=output_path / f'{ctx.project_name}GamePackage',
                                                         project_file_patterns=project_file_patterns_to_copy + game_project_file_patterns_to_copy,
                                                         ignore_file_patterns=[f'*.ServerLauncher{exp.EXECUTABLE_EXTENSION}', f'*.UnifiedLauncher{exp.EXECUTABLE_EXTENSION}']))

        if should_build_server_launcher:
            export_layouts.append(exp.ExportLayoutConfig(output_path=output_path / f'{ctx.project_name}ServerPackage',
                                                         project_file_patterns=project_file_patterns_to_copy + server_project_file_patterns_to_copy,
                                                         ignore_file_patterns=[f'*.GameLauncher{exp.EXECUTABLE_EXTENSION}', f'*.UnifiedLauncher{exp.EXECUTABLE_EXTENSION}']))

        if should_build_unified_launcher:
            export_layouts.append(exp.ExportLayoutConfig(output_path=output_path / f'{ctx.project_name}UnifiedPackage',
                                                         project_file_patterns=project_file_patterns_to_copy + game_project_file_patterns_to_copy + server_project_file_patterns_to_copy,
                                                         ignore_file_patterns=[f'*.ServerLauncher{exp.EXECUTABLE_EXTENSION}', f'*.GameLauncher{exp.EXECUTABLE_EXTENSION}']))

        for export_layout in export_layouts:
            exp.setup_launcher_layout_directory(project_path=ctx.project_path,
                                                asset_platform=selected_platform,
                                                game_build_path=game_build_path,
                                                build_config=build_config,
                                                bundles_to_copy=[expected_bundles_path / f'game_{selected_platform}.pak',
                                                                 expected_bundles_path / f'engine_{selected_platform}.pak'],
                                                export_layout=export_layout,
                                                archive_output_format=archive_output_format,
                                                logger=logger)
    finally:
        # Make sure to clean up and restore the state of the MPSGame
        if gamelift_gem_added:
            if disable_gem.disable_gem_in_project(gem_name="MPSGameLift", project_path=ctx.project_path) != 0:
                logger.warning("Unable to remove the project's 'MPSGameList' gem")
        elif gamelift_gem_removed:
            if enable_gem.enable_gem_in_project(gem_name="MPSGameLift", project_path=ctx.project_path) != 0:
                logger.warning("Unable to restore project's 'MPSGameList' gem")


# This code is only run by the 'export-project' O3DE CLI command
if "o3de_context" in globals():

    global o3de_context
    global o3de_logger

    def parse_args(o3de_context: exp.O3DEScriptExportContext):

        parser = argparse.ArgumentParser(
                    prog=f'o3de.py export-project -es {__file__}',
                    description="Exports the Multiplayer Samples project as standalone to the desired output directory with release layout. "
                                "In order to use this script, the engine and project must be setup and registered beforehand. ",
                    epilog="Note: You can pass additional arguments to the cmake command to build the projects during the export"
                           "process by adding a '/' between the arguments for this script and the additional arguments you would"
                           "like to pass down to cmake build.",
                    add_help=False
        )
        parser.add_argument(exp.CUSTOM_SCRIPT_HELP_ARGUMENT,default=False,action='store_true',help='Show this help message and exit.')
        parser.add_argument('-out', '--output-path', type=pathlib.Path, required=True, help='Path that describes the final resulting Release Directory path location.')
        parser.add_argument('-cfg', '--config', type=str, default='profile', choices=['release', 'profile'],
                            help='The CMake build configuration to use when building project binaries.  Tool binaries built with this script will always be built with the profile configuration.')
        parser.add_argument('-a', '--archive-output',  type=str,
                            help="Option to create a compressed archive the output. "
                                 "Specify the format of archive to create from the output directory. If 'none' specified, no archiving will occur.",
                            choices=["none", "zip", "gzip", "bz2", "xz"], default="none")
        parser.add_argument('-gl', '--game-lift', default=False, action='store_true',
                            help='Enable Gamelift for the Multiplayer Sample package')
        parser.add_argument('-code', '--should-build-code', default=False,action='store_true',
                            help='Toggles building all code for the project by launcher type (game, server, unified).')
        parser.add_argument('-assets', '--should-build-assets', default=False, action='store_true',
                            help='Toggles building all assets for the project by launcher type (game, server, unified).')
        parser.add_argument('-foa', '--fail-on-asset-errors', default=False, action='store_true',
                            help='Option to fail the project export process on any failed asset during asset building (applicable if --should-build-assets is true)')
        parser.add_argument('-bt', '--build-tools', default=True, type=bool,
                            help="Specifies whether to build O3DE toolchain executables. This will build AssetBundlerBatch, AssetProcessorBatch.")
        parser.add_argument('-tbp', '--tools-build-path', type=pathlib.Path, default=None,
                            help='Designates where O3DE toolchain executables go. If not specified, default is <o3de_project_path>/build/tools.')
        parser.add_argument('-gbp', '--game-build-path', type=pathlib.Path, default=None,
                            help="Designates where project executables (like Game/Server Launcher) go."
                            " If not specified, default is <o3de_project_path>/build/game.")
        parser.add_argument('-regovr', '--allow-registry-overrides', default=False, type = bool,
                            help="When configuring cmake builds, this determines if the script allows for overriding registry settings from external sources.")
        parser.add_argument('-maxsize', '--max-bundle-size', type=int, default=2048, help='Specify the maximum size of a given asset bundle.')
        parser.add_argument('-nogame', '--no-game-launcher', action='store_true', help='This flag skips building the Game Launcher on a platform if not needed.')
        parser.add_argument('-noserver', '--no-server-launcher', action='store_true', help='This flag skips building the Server Launcher on a platform if not needed.')
        parser.add_argument('-nounified', '--no-unified-launcher', action='store_true', help='This flag skips building the Unified Launcher on a platform if not needed.')
        parser.add_argument('-pl', '--platform', type=str, default=exp.get_default_asset_platform(), choices=['pc', 'linux', 'mac'])
        parser.add_argument('-q', '--quiet', action='store_true', help='Suppresses logging information unless an error occurs.')

        if o3de_context is None:
            parser.print_help()
            exit(0)
        
        parsed_args = parser.parse_args(o3de_context.args)
        if parsed_args.script_help:
            parser.print_help()
            exit(0)

        return parsed_args
    
    args = parse_args(o3de_context)
    if args.quiet:
        o3de_logger.setLevel(logging.ERROR)
    try:
        export_multiplayer_sample(ctx=o3de_context,
                                  selected_platform=args.platform,
                                  output_path=args.output_path,
                                  should_build_tools=args.build_tools,
                                  build_config=args.config,
                                  max_bundle_size=args.max_bundle_size,
                                  enable_gamelift=args.game_lift,
                                  should_build_all_code=args.should_build_code,
                                  should_build_all_assets=args.should_build_assets,
                                  fail_on_asset_errors=args.fail_on_asset_errors,
                                  should_build_game_launcher=not args.no_game_launcher,
                                  should_build_server_launcher=not args.no_server_launcher,
                                  should_build_unified_launcher=not args.no_unified_launcher,
                                  allow_registry_overrides=args.allow_registry_overrides,
                                  tools_build_path=args.tools_build_path,
                                  game_build_path=args.game_build_path,
                                  archive_output_format=args.archive_output,
                                  logger=o3de_logger)
    except exp.ExportProjectError as err:
        print(err)
        sys.exit(1)
