# O3DE MPS Project Export Testing Instructions

## Notes

* This instruction set assumes the host platform is Windows. Instructions for Linux coming shortly.

## Fresh Install and Setup
1. Clone the development branch of O3DE:
```bash
cd \path\to\workspace
git clone -b development https://github.com/o3de/o3de.git 
```
2. Clone the development branch of O3DE MPS:
```bash
git clone -b development https://github.com/o3de/o3de-multiplayersample.git
```
3. Clone O3DE MPS Assets, then switch to development branch. Afterwards initialize any submodules (such as O3DEPopcornFX), and make sure they are on development:
```bash
git clone https://github.com/o3de/o3de-multiplayersample-assets.git
cd o3de-multiplayersample-assets
git switch development
git submodule update --init --recursive
```
4. Now register the engine, project, and gems (this is similar to [registration found in O3DE MPS Github readme](https://github.com/o3de/o3de-multiplayersample/blob/MPSProjectExportTestingInstructions/README.md#step-2-register-the-engine-the-project-and-the-gems)):
```bash
cd \path\to\workspace\o3de
python\get_python
scripts\o3de register --this-engine
scripts\o3de register -p \path\to\workspace\o3de-multiplayersample
scripts\o3de register --all-gems-path \path\to\workspace\o3de-multiplayersample-assets\Gems
```

## Run Project Export
Use the O3DE Project-centric export script to produce a game and server package. 

1. Create the directory you want to output the game and server, navigate to that directory, and run the export command:
```bash
\path\to\workspace\o3de\scripts\o3de export-project -es \path\to\o3de\scripts\o3de\ExportScripts\export_standalone_monolithic_project_centric.py -pp \path\to\o3de-multiplayersample -out \full\path\to\output -cfg release -a zip -nounified -gpfp launch_client.cfg -spfp launch_client.cfg -code -assets -ll INFO -sl \path\to\o3de-multiplayersample\AssetBundling\SeedLists\BasePopcornFxSeedList.seed -sl \path\to\o3de-multiplayersample\AssetBundling\SeedLists\GameSeedList.seed -sl \path\to\o3de-multiplayersample\AssetBundling\SeedLists\VFXSeedList.seed 
```

2. You should see two directories in your output folder: `MultiplayerSampleGamePackage` and `GameLiftPackageWindows`.

## Test Exported Project
1. To test MPS, first run the server, then run the game. You may need to provide admin privilege to enable a connection to AssetProcessor:
```bash
.\GameLiftPackageWindows\MultiplayerSample.ServerLauncher.exe --rhi=null -NullRenderer --console-command-file=launch_server.cfg --net_udpDefaultTimeoutMs=20000
 
# Wait for server to get setup, then run the game launcher
.\MultiplayerSampleGamePackage\MultiplayerSample.GameLauncher.exe --connect=127.0.0.1 --net_udpDefaultTimeoutMs=20000
```

2. At this point, check to see if the game runs, and if you're able to run around, see particles, shoot, and hear sounds. Any errors or crashes should result in timestamped logs, which can be found at `GameLiftPackageWindows\user\log\server.log`, or `MultiplayerSampleGamePackage\user\log\game.log`.
