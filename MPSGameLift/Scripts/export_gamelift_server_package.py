#
# Copyright (c) Contributors to the Open 3D Engine Project.
# For complete copyright and license terms please see the LICENSE at the root of this distribution.
#
# SPDX-License-Identifier: Apache-2.0 OR MIT
#
#
"""
Enables the project for AWS Gamelift and creates a Windows server package which can be uploaded to a GameLift.

To use this script pass it into o3de.bat's export-project command: 
<path-to-o3de-engine>\scripts\o3de.bat export-project -es <path-to-multiplayer-sample>\MPSGameLift\Scripts\export_gamelift_server_package.py -ll INFO
"""

import os
import argparse

import o3de.export_project as exp
import o3de.enable_gem as enable_gem

from o3de.export_project import process_command
from o3de import manifest

project_json_data = manifest.get_project_json_data(project_path=o3de_context.project_path)
project_name = project_json_data.get('project_name')

o3de_logger.info(f"Exporting AWS GameLift Server Package for {project_name}")

# Parse arguments to either build code, assets, or both
parser = argparse.ArgumentParser(
                    prog='GameLift Server Package',
                    description='Helps setup the project for AWS Gamelift and creates a Windows server package which can be uploaded to a GameLift.')

parser.add_argument('--code', action='store_true', help='Build code')
parser.add_argument('--assets', action='store_true', help='Build assets')
parser.add_argument('-g', '--generator', choices=['Visual Studio 16', 'Visual Studio 17'], help='Which compiler do you want to use?')

args = parser.parse_args(o3de_context.args)

# Help user choose to build code, assets, or both if they didn't specify via command-line
while not args.code and not args.assets:
    user_input = input('No build command specified. Do you want to build code, assets, or both? (c/a/b). Quit(q): ')
    if user_input.lower() == 'c':
        args.code = True
    elif user_input.lower() == 'a':
        args.assets = True
    elif user_input.lower() == 'b':
        args.code = True
        args.assets = True
    elif user_input.lower() == 'q':
        quit()

# Help user choose their compiler if they didn't specify via command-line
while not args.generator:
    user_input = input('Select generator:\n 1. Visual Studio 16\n 2. Visual Studio 17.\n Quit(q)\n')
    if user_input == '1':
        args.generator = "Visual Studio 16"
    if user_input == '2':
        args.generator = "Visual Studio 17"
    elif user_input.lower() == 'q':
        quit()
        
build_folder = os.path.join(o3de_context.project_path, "build", "windows")

# Build code
if (args.code):
    # Enable GameLift gems
    o3de_logger.info(f"Enabling AWSGameLift and MPSGameLift gem")
    if (enable_gem.enable_gem_in_project(gem_name="AWSGameLift", project_path=o3de_context.project_path) != 0):
        quit()

    if (enable_gem.enable_gem_in_project(gem_name="MPSGameLift", project_path=o3de_context.project_path) != 0):
        quit()

    # Build server launcher
    os.makedirs(build_folder, exist_ok=True)
    o3de_logger.info(f"Building {project_name}.ServerLauncher")

    if (process_command(["cmake", "-B", build_folder, "-S", o3de_context.project_path, "-G", args.generator])):
        quit()

    if (process_command(["cmake", "--build", build_folder, "--target", f"{project_name}.ServerLauncher", "AssetProcessor", "AssetBundler", "AssetBundlerBatch", "--config", "profile", "--", "/m"]) != 0):
        quit()
        
    # Build monolithic server launcher build
    monolithic_build_folder = os.path.join(o3de_context.project_path, "build", "windows_mono")
    os.makedirs(monolithic_build_folder, exist_ok=True)
    if (process_command(["cmake", "-B", monolithic_build_folder, "-S", o3de_context.project_path, "-G", args.generator, "-DLY_MONOLITHIC_GAME=1", "-DALLOW_SETTINGS_REGISTRY_DEVELOPMENT_OVERRIDES=0"])):
        quit()

    if (process_command(["cmake", "--build", monolithic_build_folder, "--target", f"{project_name}.ServerLauncher", "--config", "profile", "--", "/m"])):
        quit()

# Build Assets
if (args.assets):
    # Process assets
    if (process_command(["cmake", "--build", build_folder, "--target", f"{project_name}.Assets", "--config", "profile", "--", "/m"]) != 0):
        quit()

    if (process_command(["cmake", "--build", build_folder, "--target", "AssetBundler", "AssetBundlerBatch", "--config", "profile", "--", "/m"]) != 0):
        quit()

    # Create a game asset list by using the game seed list
    platform = "pc"
    asset_bundler_batch = os.path.join(build_folder, "bin", "profile", "AssetBundlerBatch.exe")
    asset_list_directory = os.path.join(o3de_context.project_path, "AssetBundling", "AssetLists" )
    seed_list_directory = os.path.join(o3de_context.project_path, "AssetBundling", "SeedLists" )
    game_asset_list_path = os.path.join(asset_list_directory, f"game_{platform}.assetlist")
    engine_asset_list_path = os.path.join(asset_list_directory, f"engine_{platform}.assetlist")

    generate_asset_list_command = f"{asset_bundler_batch} assetLists --assetListFile {game_asset_list_path} --platform {platform} --allowOverwrites"
    
    # Add all the .seed files found inside <project>/AssetBundling/SeedLists
    seed_file_extension = ".seed"
    
    seed_files = [os.path.join(seed_list_directory, f) for f in os.listdir(seed_list_directory) if f.endswith(seed_file_extension)]

    if not seed_files:
        o3de_logger.error(f"Building assets failed! Could not find any game seed files inside {seed_list_directory}")
        quit()

    for file in seed_files:
        generate_asset_list_command += str(f" --seedListFile ")
        generate_asset_list_command += str(os.path.join(seed_list_directory, file))

    if (process_command(generate_asset_list_command.split()) != 0):
        quit()


    if (process_command([asset_bundler_batch, "assetLists", "--assetListFile", game_asset_list_path, "--platform", platform, "--allowOverwrites",
                         "--seedListFile", os.path.join(seed_list_directory, "BasePopcornFxSeedList.seed"), 
                         "--seedListFile", os.path.join(seed_list_directory, "GameSeedList.seed"), 
                         "--seedListFile", os.path.join(seed_list_directory, "ProfileOnlySeedList.seed"), 
                         "--seedListFile", os.path.join(seed_list_directory, "VFXSeedList.seed")]) != 0):
        quit()

    # Create a engine asset list by using the engine seed list
    if (process_command([asset_bundler_batch, "assetLists", "--assetListFile", engine_asset_list_path, "--platform", platform, "--allowOverwrites",
                         "--addDefaultSeedListFiles"]) != 0):
        quit()

    # Bundle game asset using game asset list
    bundles_directory = os.path.join(o3de_context.project_path, "AssetBundling", "Bundles" )
    if (process_command([asset_bundler_batch, "bundles", "--maxSize", "2048", "--platform", platform, "--allowOverwrites",
                         "--outputBundlePath", os.path.join(bundles_directory, "game.pak"),
                         "--assetListFile", game_asset_list_path]) != 0):
        quit()

    # Bundle engine asset using engine asset list
    if (process_command([asset_bundler_batch, "bundles", "--maxSize", "2048", "--platform", platform, "--allowOverwrites",
                         "--outputBundlePath", os.path.join(bundles_directory, "engine.pak"),
                         "--assetListFile", engine_asset_list_path]) != 0):
        quit()
