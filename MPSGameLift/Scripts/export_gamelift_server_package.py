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

args = parser.parse_args(o3de_context.args)

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
    else:
        print('Invalid input. Please enter c, a, or b.')

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

    if (process_command(["cmake", "-B", build_folder, "-S", o3de_context.project_path, "-G", "Visual Studio 16"])):
        quit()

    if (process_command(["cmake", "--build", build_folder, "--target", f"{project_name}.ServerLauncher", "AssetBundler", "--config", "profile", "--", "/m"]) != 0):
        quit()
        
    # Build monolithic server launcher build
    monolithic_build_folder = os.path.join(o3de_context.project_path, "build", "windows_mono")
    os.makedirs(monolithic_build_folder, exist_ok=True)
    if (process_command(["cmake", "-B", monolithic_build_folder, "-S", o3de_context.project_path, "-G", "Visual Studio 16", "-DLY_MONOLITHIC_GAME=1", "-DALLOW_SETTINGS_REGISTRY_DEVELOPMENT_OVERRIDES=0"])):
        quit()

    if (process_command(["cmake", "--build", monolithic_build_folder, "--target", f"{project_name}.ServerLauncher", "--config", "profile", "--", "/m"])):
        quit()

# Build Assets
if (args.assets):
    # Process assets
    if (process_command(["cmake", "--build", build_folder, "--target", f"{project_name}.Assets", "--config", "profile", "--", "/m"]) != 0):
        quit()
