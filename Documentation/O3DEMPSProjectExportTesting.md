# O3DE MPS Project Export Testing Instructions

## Notes

* The general O3DE project-centric export script is still in development. For now the project-centric instructions will be calibrated using the MPS Gamelift Export Script. Once the general script is merged, the instructions will be updated.
* This instruction set assumes the host platform is Windows. Instructions for Linux coming shortly.

## Fresh Install and Setup
1. Clone the development branch of O3DE:
```bash
cd \path\to\workspace
git clone https://github.com/o3de/o3de.git # this should already be in development branch
```
2. Clone the development branch of O3DE MPS:
```bash
git clone https://github.com/o3de/o3de-multiplayersample.git # this should already be in development branch
```
3. Clone O3DE MPS Assets, then switch to development branch. Afterwards initialize any submodules (such as O3DEPopcornFX), and make sure they are on development:
```bash
git clone https://github.com/o3de/o3de-multiplayersample-assets.git
cd o3de-multiplayersample-assets
git switch development
git lfs pull
git submodule update --init --recursive
cd Gems/O3DEPopcornFXPlugin
git switch development
```
4. Now get the engine registered, along with projects (this is similar to registration found in O3DE MPS Github readme):
```bash
cd \path\to\workspace\o3de
python\get_python
scripts\o3de register --this-engine
scripts\o3de register --all-gems-path \path\to\workspace\o3de-multiplayersample-assets\Gems
scripts\o3de register -p \path\to\workspace\o3de-multiplayersample
```

## Run Project Export
We can use the export script for MPSGamelift to export in a project-centric manner. 

1. To do so, first create the directory you want to output the game to, cd into that directory, then run the export command:
```bash
mkdir \path\to\output
cd \path\to\output
\full\path\to\workspace\o3de\scripts\o3de export-project -es \full\path\to\workspace\o3de-multiplayersample\MPSGameLift\Scripts\export_gamelift_server_package.py --code --assets -ll INFO --package-gamelauncher
```

2. Use `ls`  to verify that the project was exported. You should see two directories in your output folder: `MultiplayerSampleGamePackage` and `GameLiftPackageWindows`

## Test Exported Project
1. To test MPS, first run the server, then run the Game. You may need to provide admin privelege to allow talking to the AssetProcessor:
```bash
.\GameLiftPackageWindows\MultiplayerSample.ServerLauncher.exe --rhi=null -NullRenderer --console-command-file=launch_server.cfg --net_udpDefaultTimeoutMs=20000
 
# wait for server to get setup, then run the game launcher
.\MultiplayerSampleGamePackage\MultiplayerSample.GameLauncher.exe --connect=127.0.0.1 --net_udpDefaultTimeoutMs=20000
```

2. At this point, check to see if the game runs, and if you're able to run around, see particles, shoot, and hear sounds. Any errors or crashes should result in timestamped logs, which can be found at `GameLiftPackageWindows\user\log\server.log`, or `MultiplayerSampleGamePackage\user\log\game.log`.
