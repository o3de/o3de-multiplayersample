#
# Copyright (c) Contributors to the Open 3D Engine Project.
# For complete copyright and license terms please see the LICENSE at the root of https://www.github.com/o3de/o3de.
#
# SPDX-License-Identifier: Apache-2.0 OR MIT
#
#
import argparse
import pathlib
import logging
import os
import time
import glob
import sys
import platform
from o3de.validation import valid_o3de_project_json, valid_o3de_engine_json
from queue import Queue, Empty
from threading  import Thread
from typing import List
from subprocess import Popen, PIPE

logger = logging.getLogger('o3de.mps_export')
LOG_FORMAT = '[%(levelname)s] %(name)s: %(message)s'
logging.basicConfig(format=LOG_FORMAT)
# This is an export script for MPS
# this has to be a complete standalone script, b/c project export doesnt exist in main branch yet

# View the argparse parameters for options available. An example invocation:

# @<O3DE_ENGINE_ROOT_PATH>
# > python\python.cmd <O3DE_MPS_PROJECT_ROOT_PATH>\ExportScripts\export_standalone_monolithic.py -pp <O3DE_MPS_PROJECT_ROOT_PATH> -ep <O3DE_ENGINE_ROOT_PATH> -bnmt -out <MPS_OUTPUT_RELEASE_DIR_PATH> -a -aof zip

def enqueue_output(out, queue):
    for line in iter(out.readline, b''):
        queue.put(line)
    out.close()

def safe_kill_processes(*processes: List[Popen], process_logger: logging.Logger = None) -> None:
    """
    Kills a given process without raising an error
    :param processes: An iterable of processes to kill
    :param process_logger: (Optional) logger to use
    """
    def on_terminate(proc) -> None:
        try:
            process_logger.info(f"process '{proc.args[0]}' with PID({proc.pid}) terminated with exit code {proc.returncode}")
        except Exception:  # purposefully broad
            process_logger.error("Exception encountered with termination request, with stacktrace:", exc_info=True)

    if not process_logger:
        process_logger = logger
    
    for proc in processes:
        try:
            process_logger.info(f"Terminating process '{proc.args[0]}' with PID({proc.pid})")
            proc.kill()
        except Exception:  # purposefully broad
            process_logger.error("Unexpected exception ignored while terminating process, with stacktrace:", exc_info=True)
    try:
        for proc in processes:
            proc.wait(timeout=30)
            on_terminate(proc)
    except Exception:  # purposefully broad
        process_logger.error("Unexpected exception while waiting for processes to terminate, with stacktrace:", exc_info=True)

class CLICommand(object):
    """
    CLICommand is an interface for storing CLI commands as list of string arguments to run later in a script.
    A current working directory, pre-existing OS environment, and desired logger can also be specified.
    To execute a command, use the run() function.
    This class is responsible for starting a new process, polling it for updates and logging, and safely terminating it.
    """
    def __init__(self, 
                args: list,
                cwd: pathlib.Path,
                logger: logging.Logger,
                env: os._Environ=None) -> None:
        self.args = args
        self.cwd = cwd
        self.env = env
        self.logger = logger
        self._stdout_lines = []
        self._stderr_lines = []
    
    @property
    def stdout_lines(self) -> List[str]:
        """The result of stdout, separated by newlines."""
        return self._stdout_lines

    @property
    def stdout(self) -> str:
        """The result of stdout, as a single string."""
        return "\n".join(self._stdout_lines)

    @property
    def stderr_lines(self) -> List[str]:
        """The result of stderr, separated by newlines."""
        return self._stderr_lines

    @property
    def stderr(self) -> str:
        """The result of stderr, as a single string."""
        return "\n".join(self._stderr_lines)

    def _poll_process(self, process, queue) -> None:
        # while process is not done, read any log lines coming from subprocess
        while process.poll() is None:
            #handle readline in a non-blocking manner
            try:  line = queue.get_nowait() 
            except Empty:
                pass
            else: # got line
                if not line: break
                log_line = line.decode('utf-8', 'ignore')
                self._stdout_lines.append(log_line)
                self.logger.info(log_line)
    
    def _cleanup_process(self, process, queue) -> str:
        # flush remaining log lines
        while not queue.empty():
            try: line = queue.get_nowait()
            except Empty:
                pass
            else:
                if not line: break
                log_line = line.decode('utf-8', 'ignore')
                self._stdout_lines.append(log_line)
                self.logger.info(log_line)
        stderr = process.stderr.read()

        safe_kill_processes(process, process_logger = self.logger)

        return stderr
    
    def run(self) -> int:
        """
        Takes the arguments specified during CLICommand initialization, and opens a new subprocess to handle it.
        This function automatically manages polling the process for logs, error reporting, and safely cleaning up the process afterwards.
        :return return code on success or failure 
        """
        ret = 1
        try:
            with Popen(self.args, cwd=self.cwd, env=self.env, stdout=PIPE, stderr=PIPE) as process:
                self.logger.info(f"Running process '{self.args[0]}' with PID({process.pid}): {self.args}")

                q = Queue()
                t = Thread(target=enqueue_output, args=(process.stdout, q))
                t.daemon = True
                t.start()

                process.stdout.flush()
                self._poll_process(process, q)
                stderr = self._cleanup_process(process, q)

                ret = process.returncode

                # print out errors if there are any      
                if stderr:
                    # bool(ret) --> if the process returns a FAILURE code (>0)
                    logger_func = self.logger.error if bool(ret) else self.logger.warning
                    err_txt = stderr.decode('utf-8', 'ignore')
                    logger_func(err_txt)
                    self._stderr_lines = err_txt.split("\n")
        except Exception as err:
            self.logger.error(err)
            raise err
        return ret

# Helper API
def process_command(args: list,
                    cwd: pathlib.Path = None,
                    env: os._Environ = None) -> int:
    """
    Wrapper for subprocess.Popen, which handles polling the process for logs, reacting to failure, and cleaning up the process.
    :param args: A list of space separated strings which build up the entire command to run. Similar to the command list of subprocess.Popen
    :param cwd: (Optional) The desired current working directory of the command. Useful for commands which require a differing starting environment.
    :param env: (Optional) Environment to use when processing this command.
    :return the exit code of the program that is run or 1 if no arguments were supplied
    """
    if len(args) == 0:
        logging.error("function `process_command` must be supplied a non-empty list of arguments")
        return 1
    return CLICommand(args, cwd, logging.getLogger(), env=env).run()


# EXPORT SCRIPT STARTS HERE!
if __name__ == "__main__":
    parser = argparse.ArgumentParser(prog='Exporter for MultiplayerSample as a standalone build',
                                 description = "Exports O3DE's MultiplayerSample to the desired output directory with release layout. "
                                                "In order to use this script, the engine and project must be setup and registered beforehand. "
                                                "See this example on the MPS Github page: "
                                                "https://github.com/o3de/o3de-multiplayersample/blob/development/README.md#required-step-to-compile")
    parser.add_argument('-pp', '--project-path', type=pathlib.Path, required=True, help='Path to the intended O3DE project.')
    parser.add_argument('-ep', '--engine-path', type=pathlib.Path, required=True, help='Path to the intended O3DE engine.')
    parser.add_argument('-out', '--output-path', type=pathlib.Path, required=True, help='Path that describes the final resulting Release Directory path location.')
    parser.add_argument('-cfg', '--config', type=str, default='profile', choices=['release', 'profile'],
                        help='The CMake build configuration to use when building project binaries. If tool binaries are built with this script, they will use profile mode.')
    parser.add_argument('-ll', '--log-level', default='ERROR',
                        choices=['DEBUG', 'INFO', 'WARNING', 'ERROR', 'CRITICAL'],
                        help="Set the log level")
    parser.add_argument('-aof', '--archive-output-format',
                        type=str,
                        help="Format of archive to create from the output directory",
                        choices=["zip", "gzip", "bz2", "xz"], default="zip")
    parser.add_argument('-bnmt', '--build-non-mono-tools', action='store_true')
    parser.add_argument('-nmbp', '--non-mono-build-path', type=pathlib.Path, default=None)
    parser.add_argument('-mbp', '--mono-build-path', type=pathlib.Path, default=None)
    parser.add_argument('-nogame', '--no-game-launcher', action='store_true', help='this flag skips building the Game Launcher on a platform if not needed.')
    parser.add_argument('-noserver', '--no-server-launcher', action='store_true', help='this flag skips building the Server Launcher on a platform if not needed.')
    parser.add_argument('-nounified', '--no-unified-launcher', action='store_true', help='this flag skips building the Unified Launcher on a platform if not needed.')
    parser.add_argument('-pl', '--platform', type=str, default=None, choices=['pc', 'linux', 'mac'])
    parser.add_argument('-a', '--archive-output', action='store_true', help='This option places the final output of the build into a compressed archive')
    parser.add_argument('-q', '--quiet', action='store_true', help='Suppresses logging information unless an error occurs.')
    args = parser.parse_args()

    if args.quiet:
        logging.getLogger().setLevel(logging.ERROR)
    else:    
        logging.getLogger().setLevel(args.log_level)

    non_mono_build_path = (args.engine_path) / 'build' / 'non_mono' if args.non_mono_build_path is None else args.non_mono_build_path
    mono_build_path = (args.engine_path) / 'build' / 'mono' if args.mono_build_path is None else args.mono_build_path

    #validation
    assert valid_o3de_project_json(args.project_path / 'project.json') and valid_o3de_engine_json(args.engine_path / 'engine.json')


    #commands are based on 
    #https://github.com/o3de/o3de-multiplayersample/blob/development/Documentation/PackedAssetBuilds.md
    selected_platform = args.platform

    system_platform = platform.system().lower()

    if not selected_platform:
        logger.info("Platform not specified! Defaulting to Host platform...")
        if not system_platform:
            logger.error("Unable to identify host platform! Please supply the platform using '--platform'. Options are [pc, linux, mac].")
            sys.exit(1)
        if system_platform == "windows":
            selected_platform = "pc"
        elif system_platform == "linux":
            selected_platform = "linux"
        elif system_platform == "darwin":
            selected_platform = "mac"
        else:
            logger.error(f"MPS exporting for {system_platform} is currently unsupported! Please use either a Windows, Mac or Linux machine to build project.")
            sys.exit(1)

    logger.info(f"Project path for MPS: {args.project_path}")
    logger.info(f"Engine path to build MPS: {args.engine_path}")
    logger.info(f"Build path for non-monolithic executables: {args.non_mono_build_path}")
    logger.info(f"Build path for monolithic executables: {args.mono_build_path}")
    
    output_cache_path = args.output_path / 'Cache' / selected_platform
    output_aws_gem_path = args.output_path / 'Gems' / 'AWSCore'
    
    os.makedirs(output_cache_path, exist_ok=True)
    os.makedirs(output_aws_gem_path, exist_ok=True)
    
    #Build o3de-multiplayersample and the engine (non-monolithic)
    if args.build_non_mono_tools:
        process_command(['cmake', '-S', '.', '-B', str(non_mono_build_path), '-DLY_MONOLITHIC_GAME=0', f'-DLY_PROJECTS={args.project_path}'], cwd=args.engine_path)

        process_command(['cmake', '--build', str(non_mono_build_path), '--target', 'AssetBundler', 'AssetBundlerBatch', 'AssetProcessor', 'AssetProcessorBatch', '--config','profile'], cwd=args.engine_path)

        process_command(['cmake', '--build', str(non_mono_build_path), '--target', 'MultiplayerSample.Assets', '--config', 'profile'], cwd=args.engine_path)
    
    #Build monolithic game
    process_command(['cmake', '-S', '.', '-B', str(mono_build_path), '-DLY_MONOLITHIC_GAME=1', '-DALLOW_SETTINGS_REGISTRY_DEVELOPMENT_OVERRIDES=0', f'-DLY_PROJECTS={args.project_path}'], cwd=args.engine_path)
    
    if not args.no_game_launcher:
        process_command(['cmake', '--build', str(mono_build_path), '--target', 'MultiplayerSample.GameLauncher', '--config', args.config], cwd=args.engine_path) 
    
    if not args.no_server_launcher:
        process_command(['cmake', '--build', str(mono_build_path), '--target', 'MultiplayerSample.ServerLauncher', '--config', args.config], cwd=args.engine_path)

    if not args.no_unified_launcher:
        process_command(['cmake', '--build', str(mono_build_path), '--target', 'MultiplayerSample.UnifiedLauncher', '--config', args.config], cwd=args.engine_path)

    #Before bundling content, make sure that the necessary executables exist
    asset_bundler_batch_path = non_mono_build_path / 'bin' / 'profile' / ('AssetBundlerBatch' + ('.exe' if system_platform=='windows' else ''))
    if not asset_bundler_batch_path.is_file():
        logger.error(f"AssetBundlerBatch not found at path '{asset_bundler_batch_path}'. In order to bundle the data for MPS, this executable must be present!")
        logger.error("To correct this issue, do 1 of the following: "
                     "1) Use the --build-non-mono-tools flag in the CLI parameters"
                     "2) If you are trying to build in a project-centric fashion, please switch to engine-centric for this export script"
                     f"3) Build AssetBundlerBatch by hand and make sure it is available at {asset_bundler_batch_path}"
                     "4) Set the --non-mono-build-path to point at a directory which contains this executable")
        sys.exit(1)

    #Bundle content
    engine_asset_list_path = args.project_path / 'AssetBundling' /  'AssetLists' / f'engine_{selected_platform}.assetlist'
    
    process_command([asset_bundler_batch_path, 'assetLists','--addDefaultSeedListFiles', '--assetListFile', engine_asset_list_path, '--project-path', args.project_path, '--allowOverwrites' ], cwd=args.engine_path)


    game_asset_list_path = args.project_path /'AssetBundling'/'AssetLists'/ f'game_{selected_platform}.assetlist'
    seed_folder_path = args.project_path/'AssetBundling'/'SeedLists'

    game_asset_list_command = [asset_bundler_batch_path, 'assetLists', '--assetListFile', game_asset_list_path, 
                    '--seedListFile', seed_folder_path  / 'BasePopcornFxSeedList.seed',
                    '--seedListFile', seed_folder_path  / 'GameSeedList.seed']

    if args.config == 'profile':
        # Dev branch has removed the profile seed list, but it still remains in main for now.
        # This will be removed after the next release, when both branches are synchronized
        profile_seed_list_path = seed_folder_path / 'ProfileOnlySeedList.seed'
        if profile_seed_list_path.is_file():
            game_asset_list_command += ['--seedListFile', profile_seed_list_path]

    game_asset_list_command += ['--seedListFile', seed_folder_path / 'VFXSeedList.seed', '--project-path', args.project_path, '--allowOverwrites']

    process_command(game_asset_list_command, cwd=args.engine_path)

    engine_bundle_path = output_cache_path / f'engine_{selected_platform}.pak'
    process_command([asset_bundler_batch_path, 'bundles', '--assetListFile', engine_asset_list_path, '--outputBundlePath', engine_bundle_path, '--project-path', args.project_path, '--allowOverwrites'], cwd=args.engine_path)

    # This is to prevent any accidental file locking mechanism from failing subsequent bundling operations
    time.sleep(1)

    game_bundle_path = output_cache_path / f'game_{selected_platform}.pak'
    process_command([asset_bundler_batch_path, 'bundles', '--assetListFile', game_asset_list_path, '--outputBundlePath', game_bundle_path, '--project-path', args.project_path, '--allowOverwrites'], cwd=args.engine_path)

    # Create Launcher Layout Directory
    import shutil

    for file in glob.glob(str(pathlib.PurePath(mono_build_path / 'bin' / args.config / '*.*'))):
        shutil.copy(file, args.output_path)
    for file in glob.glob(str(pathlib.PurePath(mono_build_path / 'bin' / args.config / 'Gems' / 'AWSCore' / '*.*'))):
        shutil.copy(file, output_aws_gem_path)
    for file in glob.glob(str(pathlib.PurePath(args.project_path / 'launch_*.*'))):
        shutil.copy(file, args.output_path)

    # Optionally zip the layout directory if the user requests
    if args.archive_output:
        archive_name = args.output_path
        logger.info("Archiving output directory (this may take a while)...")
        shutil.make_archive(args.output_path, args.archive_output_format, root_dir = args.output_path)

    logger.info(f"Exporting project is complete! Release Directory can be found at {args.output_path}")
