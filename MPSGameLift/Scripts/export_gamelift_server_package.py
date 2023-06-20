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
A folder named GameLiftPackageWindows containing the server will be created. 
After this script runs, test the server locally and upload the server package to GameLift. 
    'example: aws gamelift upload-build --server-sdk-version 5.0.0 --operating-system WINDOWS_2016 --build-root .\GameLiftPackageWindows\ --name MultiplayerSample --build-version v1.0 --region us-west-2'
"""

import os
import argparse
import shutil
import psutil

import o3de.export_project as exp
import o3de.enable_gem as enable_gem

from o3de.export_project import process_command
from o3de import manifest

project_json_data = manifest.get_project_json_data(project_path=o3de_context.project_path)
project_name = project_json_data.get('project_name')
build_folder = os.path.join(o3de_context.project_path, "build", "windows")
monolithic_build_folder = os.path.join(o3de_context.project_path, "build", "windows_mono")
bundles_directory = os.path.join(o3de_context.project_path, "AssetBundling", "Bundles" )
gamelift_package_folder_name = "GameLiftPackageWindows"
client_package_folder_name = "MultiplayerSampleGamePackage"

o3de_logger.info(f"Exporting AWS GameLift Server Package for {project_name}")

# Parse arguments to either build code, assets, or both
parser = argparse.ArgumentParser(
                    prog='GameLift Server Package',
                    description='Helps setup the project for AWS Gamelift and creates a Windows server package which can be uploaded to a GameLift.')

parser.add_argument('--code', action='store_true', help='Build code')
parser.add_argument('--assets', action='store_true', help='Build assets')
parser.add_argument('--package-gamelauncher', action='store_true', help='Produce a client GameLauncher.exe release package along side the GameLift server package.')
parser.add_argument('-g', '--generator', choices=['Visual Studio 16', 'Visual Studio 17'], help='Which compiler do you want to use?')
parser.add_argument('--no-clobber', action='store_true', dest='no_clobber', help='Do not create a new package if an existing GameLift server package exists.')

args = parser.parse_args(o3de_context.args)

# Ask user to shutdown any O3DE applications before building
o3de_process_names = ['o3de', 'editor', 'assetprocessor', f'{project_name.lower()}.serverlauncher', f'{project_name.lower()}.gamelauncher' ]
for process in psutil.process_iter():
    # strip off .exe and check process name
    if os.path.splitext(process.name())[0].lower() in o3de_process_names:
        user_input = input(f'{process.name()} is running. Continuing may cause build errors.\nStop {process.name()} before continuing? (y/n). Quit(q)')
        if user_input.lower() == 'y':
            process.terminate()
            process.wait()
        elif user_input.lower() == 'q':
            quit()

# Check if the GameLift server package folder already exists
if os.path.exists(gamelift_package_folder_name) and args.no_clobber:
    print(f"{gamelift_package_folder_name} folder already exists. Respecting --no-clobber and exiting. No new package created.")
    exit()

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
    user_input = input('Select generator:\n 1. Visual Studio 16 (2019)\n 2. Visual Studio 17 (2022).\n Quit(q)\n')
    if user_input == '1':
        args.generator = "Visual Studio 16"
    if user_input == '2':
        args.generator = "Visual Studio 17"
    elif user_input.lower() == 'q':
        quit()

# Build code
if (args.code):
    # Enable GameLift gems
    o3de_logger.info(f"Enabling MPSGameLift gem")
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
    os.makedirs(monolithic_build_folder, exist_ok=True)
    if (process_command(["cmake", "-B", monolithic_build_folder, "-S", o3de_context.project_path, "-G", args.generator, "-DLY_MONOLITHIC_GAME=1", "-DALLOW_SETTINGS_REGISTRY_DEVELOPMENT_OVERRIDES=0"])):
        quit()

    if (process_command(["cmake", "--build", monolithic_build_folder, "--target", f"{project_name}.ServerLauncher", "--config", "release", "--", "/m"])):
        quit()

    # Buld the monolithic game launcher build
    if args.package_gamelauncher:
        if (process_command(["cmake", "--build", monolithic_build_folder, "--target", f"{project_name}.GameLauncher", "--config", "release", "--", "/m"])):
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
                         "--seedListFile", os.path.join(seed_list_directory, "VFXSeedList.seed")]) != 0):
        quit()

    # Create a engine asset list by using the engine seed list
    if (process_command([asset_bundler_batch, "assetLists", "--assetListFile", engine_asset_list_path, "--platform", platform, "--allowOverwrites",
                         "--addDefaultSeedListFiles"]) != 0):
        quit()

    # Bundle game asset using game asset list
    if (process_command([asset_bundler_batch, "bundles", "--maxSize", "2048", "--platform", platform, "--allowOverwrites",
                         "--outputBundlePath", os.path.join(bundles_directory, "game.pak"),
                         "--assetListFile", game_asset_list_path]) != 0):
        quit()

    # Bundle engine asset using engine asset list
    if (process_command([asset_bundler_batch, "bundles", "--maxSize", "2048", "--platform", platform, "--allowOverwrites",
                         "--outputBundlePath", os.path.join(bundles_directory, "engine.pak"),
                         "--assetListFile", engine_asset_list_path]) != 0):
        quit()

def create_exe_package(new_package_folder_name, exe_name):
    # Add all the required exe's, dll's, and asset pak files into a folder to upload to GameLift
    package_cache_dir = os.path.join(new_package_folder_name, "Cache", "pc")

    # Delete the old server package
    if os.path.exists(new_package_folder_name):
        shutil.rmtree(new_package_folder_name)

    # Create the folders
    os.makedirs(package_cache_dir, exist_ok=True)

    # Copy .exe and .dll files to GameLiftWindowsServerPackage directory
    build_dir = os.path.join(monolithic_build_folder, "bin", "release")
    for file_name in os.listdir(build_dir):
        file_path = os.path.join(build_dir, file_name)
        if os.path.isfile(file_path) and file_name.lower().endswith((exe_name.lower(), '.dll')):
            shutil.copy2(file_path, new_package_folder_name)

    # Copy .pak files to Cache\pc directory
    for file_name in os.listdir(bundles_directory):
        if file_name.endswith(".pak"):
            file_path = os.path.join(bundles_directory, file_name)
            shutil.copy2(file_path, package_cache_dir)

# Create the GameLift server package
create_exe_package(gamelift_package_folder_name, 'ServerLauncher.exe')

# GameLift server package needs launch_server.cfg file
launch_server_cfg_filepath = os.path.join(o3de_context.project_path, "launch_server.cfg")
if os.path.isfile(launch_server_cfg_filepath):
    shutil.copy(launch_server_cfg_filepath, gamelift_package_folder_name)
else:
    o3de_logger.error(f"Could not find serverlauncher.cfg! Launch_server.cfg is required because there's a bug with multiplayer when calling --loadlevel in the command-line. See https://github.com/o3de/o3de/issues/15773.")
    quit()

# GameLift server needs AWSCore metadata files that have been output to the build directory.
gamelift_package_gems_dir = os.path.join(gamelift_package_folder_name, "Gems", "AWSCore")
os.makedirs(gamelift_package_gems_dir, exist_ok=True)
gems_files_dir = os.path.join(monolithic_build_folder, "bin", "release", "Gems", "AWSCore")
for file_name in os.listdir(gems_files_dir):
    file_path = os.path.join(gems_files_dir, file_name)
    if os.path.isfile(file_path):
        shutil.copy(file_path, gamelift_package_gems_dir)

# Create the Client Package
if args.package_gamelauncher:
    create_exe_package(client_package_folder_name, 'GameLauncher.exe')

o3de_logger.info("Export Successful!\n"
                 "Test the server locally and upload the server package to GameLift.\n"
                 f"Example: aws gamelift upload-build --server-sdk-version 5.0.0 --operating-system WINDOWS_2016 --build-root .\GameLiftPackageWindows\ --name {project_name} --build-version v1.0 --region us-west-2")
