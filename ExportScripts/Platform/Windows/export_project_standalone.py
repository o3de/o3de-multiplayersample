#
# Copyright (c) Contributors to the Open 3D Engine Project.
# For complete copyright and license terms please see the LICENSE at the root of this distribution.
#
# SPDX-License-Identifier: Apache-2.0 OR MIT
#
#
from o3de.export_project import *
from o3de import manifest
import argparse
import pathlib
import os
import shutil

#Note: project_path and engine_path are computed before this script runs, and can be accessed via o3de_context.project_path
#for now this script assumes an engine-centric approach, and we assume to use the source built engine for the engine_path

#to use this script, go to the engine root directory, and issue the following command:
#.\scripts\o3de.bat export-project -pp C:\path\to\project -es C:\path\to\export_profile_monolithic.py -nmp C:\path\to\build\non_mono -mp C:\path\to\build\mono -out C:\path\to\output -ll INFO

parser = argparse.ArgumentParser(prog="Project exporter for windows",
                                 description="Exports a packaged build for the project")
parser.add_argument('-nmp', '--non-mono-build-path', type=pathlib.Path, required=True)
parser.add_argument('-mp', '--mono-build-path',type=pathlib.Path, required=True)
parser.add_argument('-nmc', '--non-mono-config', type=str, default='profile', choices=['profile', 'release', 'debug'], help='specifies the build configuration to use for the non-monolithic build of the tool artifacts')
parser.add_argument('-mc', '--mono-config', type=str, default='profile', choices=['profile', 'release', 'debug'], help='specifies the build configuration to use for the monolithic build of the launcher artifacts')
parser.add_argument('-r', '--is-release', action='store_true')
parser.add_argument('-out', '--output-path', type=pathlib.Path, required=True)


default_asset_type = 'pc'
if sys.platform == 'win32':
    default_asset_type = 'pc'
elif sys.platform.startswith('linux'):
    default_asset_type = 'linux'
elif sys.platform.startswith('darwin'):
    default_asset_type = 'mac'
else:
    o3de_context.logger.error(f'Export script for platform {sys.platform} does not have a default asset type.' \
        " Using 'pc' as a fallback.")
parser.add_argument('-pat', '--platform-asset-type', type=str, default= default_asset_type, choices=['pc, linux, mac'])

parser.set_defaults(is_release=False)
args = parser.parse_args(o3de_context.args)

projects_path_cmake_option = f"-DLY_PROJECTS={str(o3de_context.project_path)}"

project_json_data = manifest.get_project_json_data(project_path= o3de_context.project_path)
project_name = project_json_data.get('project_name')

if not project_name:
    project_name = os.path.basename(o3de_context.project_path)

#clean the output folder if necessary
if os.path.isdir(args.output_path):
    shutil.rmtree(args.output_path)



cmake_non_mono_build_folder = args.non_mono_build_path

if not os.path.isabs(str(cmake_non_mono_build_folder)):
    cmake_non_mono_build_folder = o3de_context.project_path / args.non_mono_build_path

cmake_mono_build_folder = args.mono_build_path

if not os.path.isabs(str(cmake_mono_build_folder)):
    cmake_mono_build_folder = o3de_context.project_path / args.mono_build_path

#build non-monolithic artifacts
process_command(['cmake', '-S', o3de_context.project_path, '-B', cmake_non_mono_build_folder], cwd= o3de_context.project_path)

process_command(['cmake', '--build', cmake_non_mono_build_folder, '--target', 'Editor', 'AssetBundler', 'AssetProcessorBatch', '--config', args.non_mono_config], cwd = o3de_context.engine_path)

executables_dir = cmake_non_mono_build_folder / 'bin' / args.non_mono_config

asset_processor_exe = executables_dir / 'AssetProcessorBatch'

process_command([asset_processor_exe, '--regset="/O3DE/Autoexec/ConsoleCommands/bg_ConnectToAssetProcessor=false"'], cwd= o3de_context.project_path)


#build monolithic artifacts
process_command(['cmake', '-S', '.', '-B', cmake_mono_build_folder, '-DLY_MONOLITHIC_GAME=1', '-DALLOW_SETTINGS_REGISTRY_DEVELOPMENT_OVERRIDES=0', '-DLY_ARCHIVE_FILE_SEARCH_MODE=2', projects_path_cmake_option], cwd= o3de_context.engine_path)
process_command(['cmake', '--build',  cmake_mono_build_folder, '--target', f'{project_name}.GameLauncher', '--target', f'{project_name}.ServerLauncher', '--target', f'{project_name}.UnifiedLauncher', '--config', args.mono_config])

#copy artifacts to output directory
# pc_pak_path = args.output_path / 'Cache' / args.platform_asset_type

process_command(['cmake', '--install', f'{cmake_mono_build_folder}', '--prefix', f'{args.output_path}', '--config', args.mono_config])