# Packaged MultiplayerSample Builds

To make relocatable client and server builds for the MultiplayerSample, we recommend making packaged builds. These package builds will contain the Game or Server Launcher and the bundled assets needed to run the launcher outside of the developer environment.

You can make both release packaged builds or profile packaged builds. For more information about creating release builds, see the O3DE documentation on [Creating a Project Game Release Layout for Windows](https://www.o3de.org/docs/user-guide/packaging/windows-release-builds/).

The guide below covers how to make profile packaged builds which are very useful for early sharing and play testing.

## Brief outline of the packaging steps

1. Install Wwise SDK (required for o3de-multiplayersample but optional for other projects)
1. Use the engine from GitHub, not the installer
1. Compile o3de-multiplayersample and the engine (non-monolithic)
1. Process assets using the Asset Processor
1. Create asset bundles (.pak files) from asset seed lists (a list of game assets)
1. Compile o3de-multiplayersample and the engine (monolithic profile or release)
1. Copy the game binaries and asset bundles to a release folder you can share

## Pre-requisites

*Important*: you can not use an installer to produce packaged builds. In order to produce a packaged build, you will need to build the engine from source from GitHub.

Additionally, we will need to compile the engine in non-monolithic profile build and in monolithic profile (and optionally release) mode.

> Note, a monolithic build is a special build of O3DE engine and projects that combines all the gems used by the project into a single game executable. Additionally, monolithic builds do not build the Editor or the Asset Processor. Thus, a regular non-monolithic build is used to process assets and bundle them into .pak files, while a monolithic build is used to produce share-able game binaries.

This guide uses `C:\git\o3de` is the source for the O3DE, cloned from GitHub.
`C:\git\o3de-multiplayersample` is the location of the clone project. These paths are optional. If you choose to use different paths, amend the command scripts in the rest of the guide accordingly.

## Windows Profile PAK Setup

Multiplayer Sample uses Wwise gem and assets for audio effects. O3DE engine (and O3DE installers) do not include Wwise support by default. In order to add the Wwise support in the engine, one must first install Wwise SDK and then re-build O3DE engine from source.

### Install Wwise
Go to https://www.audiokinetic.com/download/, create a login, log in, and download the installer. Wwise is needed to process and package audio assets in the project.

![Wwise installer options](Media/Wwise_installer_options.png)

Inside the installer select the version to use.  Install version **2021.1.11.7933**, select both Authoring and SDK, Microsoft platform.

![Wwise version selection](Media/Wwise_installer_version_selection.png)

> REBOOT (or logout / login). Otherwise, the environment settings won't get picked up for any builds in Visual Studio. They will only apply to command-line builds, and only for any command-line windows that have been opened after the installer finishes.

### Build o3de-multiplayersample and the engine (non-monolithic)

Build a regular profile build of the game as per the [README.md](../README.md) in an engine-centric way. Be sure to build from the source engine and not the installer. Here is an example:

Clone the engine from source.

```shell
C:\git> git clone https://github.com/o3de/o3de
```

Navigate to `C:\git\o3de`.
Create a build folder for non-monolithic build of the engine, for example `C:\git\o3de\build\non_mono`.

> The location and the name of the build folder is optional.

Configure the engine in a non-monolithic mode with o3de-multiplayersample project.

```shell
C:\git\o3de> cmake -S . -B build\non_mono -DLY_MONOLITHIC_GAME=0 -DLY_PROJECTS="C:\git\o3de-multiplayersample"
```

> `-DLY_MONOLITHIC_GAME=0` is the default value but for clarity it's specified here explicitly.

Build the Editor and the Asset Bundler. This will compile the project and necessary gems to produce the required game assets.

```shell
C:\git\o3de> cmake --build build\non_mono --target Editor --target AssetBundler --config profile
```

> You will need to build the [AssetBundler](https://www.o3de.org/docs/user-guide/packaging/asset-bundler/overview/) tool to create game bundles (.pak files).

Run the Asset Processor with o3de-multiplayersample and let all the assets get processed.

```shell
C:\git\o3de> .\build\non_mono\bin\profile\AssetProcessor.exe --project-path C:\git\o3de-multiplayersample
```

### Build monolithic game

Build a second version of the executables as monolithic pak builds.

> A separate build folder is required for building monolithic binaries, separate from the non-monolithic build folder.

```shell
# Create build files for a monolithic build that also disables all user/project registry settings overrides
C:\git\o3de> cmake -S . -B build\mono -DLY_MONOLITHIC_GAME=1 -DALLOW_SETTINGS_REGISTRY_DEVELOPMENT_OVERRIDES=0 -DLY_PROJECTS="C:\git\o3de-multiplayersample"

# Build the profile versions of all the executables
C:\git\o3de> cmake --build build\mono --target MultiplayerSample.GameLauncher MultiplayerSample.ServerLauncher MultiplayerSample.UnifiedLauncher --config profile
```

Profile monolithic game binaries will be located in `C:\git\o3de\build\mono\bin\profile`.
Optionally, you can build monolithic release game binaries.

> Release game binaries won't have to access to a developer console and/or certain logs. It is a good idea to get profile monolithic build to work before testing the release monolithic binaries.

```shell
C:\git\o3de> cmake --build build\mono --target MultiplayerSample.GameLauncher MultiplayerSample.ServerLauncher MultiplayerSample.UnifiedLauncher --config release
```

Release monolithic game binaries will be located in `C:\git\o3de\build\mono\bin\release`. The contents of these folders can be copied and run anywhere, once the game bundles (.pak files) are put in the proper location.


### Bundle Content

Run the AssetBundler

```shell
build\non_mono\bin\profile\AssetBundler.exe --project-path="c:\your\path\to\o3de-multiplayersample"
```

Follow steps for "Create a bundle for game assets", "Create a bundle for engine assets" and "Add bundles to the release game layout" from https://www.o3de.org/docs/user-guide/packaging/asset-bundler/bundle-assets-for-release/

* The "default seed lists" choice should choose all but 3 seed lists to make the `engine_pc.pak`
* The other [seed lists](https://github.com/o3de/o3de-multiplayersample/tree/development/AssetBundling/SeedLists) should all get selected to make the `game_pc.pak`.

> It's important to make sure that the bootstrap.game.profile.setreg file has been added to one of the seed files. (also add debug if you want to support debug builds)

### Using AssetBundlerBatch Command Line Interface to Bundle Content

Alternatively, you can use the command line batch tool (from profile non-monolithic build) to bundle the assets. The bundling process is as follows:

* Generate asset lists from the existing seed files that reference the game assets used by o3de-multiplayersample. (The seed files can be found at `\path\to\o3de-multiplayersample\AssetBundling\SeedLists`.)

```shell
build\windows\bin\profile\AssetBundlerBatch.exe assetLists 
    --addDefaultSeedListFiles 
    --assetListFile \path\to\o3de-multiplayersample\AssetBundling\AssetLists\engine_pc.assetlist 
    --project-path \path\to\o3de-multiplayersample 
    --allowOverwrites
```

> O3DE gems specify their default seed lists by placing a seed file at `\path\to\o3de\Gems\Terrain\Assets\seedList.seed`. If a project references a gem these default seed lists will be included with option `--addDefaultSeedListFiles`.

Project custom seed lists can be specified as follows:

```shell
build\windows\bin\profile\AssetBundlerBatch.exe assetLists 
    --assetListFile \path\to\o3de-multiplayersample\AssetBundling\AssetLists\game_pc.assetlist 
    --seedListFile \path\to\o3de-multiplayersample\AssetBundling\SeedLists\BasePopcornFxSeedList.seed 
    --seedListFile \path\to\o3de-multiplayersample\AssetBundling\SeedLists\GameSeedList.seed 
    --seedListFile \path\to\o3de-multiplayersample\AssetBundling\SeedLists\VFXSeedList.seed 
    --project-path \path\to\o3de-multiplayersample 
    --allowOverwrites
```

> `--allowOverwrites` is an optional parameter to allow overwriting existing asset lists.

* Use generated asset lists to generate game bundles.

```shell
build\windows\bin\profile\AssetBundlerBatch.exe bundles 
    --assetListFile \path\to\o3de-multiplayersample\AssetBundling\AssetLists\engine_pc.assetlist 
    --outputBundlePath \path\to\o3de-multiplayersample\AssetBundling\Bundles\engine_pc.pak 
    --project-path \path\to\o3de-multiplayersample 
    --allowOverwrites
```

```shell
build\windows\bin\profile\AssetBundlerBatch.exe bundles 
    --assetListFile \path\to\o3de-multiplayersample\AssetBundling\AssetLists\game_pc.assetlist 
    --outputBundlePath \path\to\o3de-multiplayersample\AssetBundling\Bundles\game_pc.pak 
    --project-path \path\to\o3de-multiplayersample 
    --allowOverwrites
```

The above commands will create two bundle files: `engine_pc.pak` and `game_pc.pak` in `\path\to\o3de-multiplayersample\AssetBundling\Bundles` folder. These pak files will need to be copied along with the monolithic game launchers to create a stand-alone final game build.

For 23.05 build of o3de-multiplayersample, the expected sizes of these pack files on Windows are:
* `engine_pc.pak` - around 72 MB
* `game_pc.pak` - around 1046 MB

### Create the Launcher Zip file

Use the following .bat file or equivalent copy steps to create a directory with the launchers in it:
```shell
rem Use this by calling 'make_release D:\my\output\dir' to make a release directory
mkdir %1
mkdir %1\Cache
mkdir %1\Cache\pc
mkdir %1\Gems
mkdir %1\Gems\AWSCore
 
rem Copy the pak files
copy c:\your\path\to\o3de-multiplayersample\AssetBundling\Bundles\*.pak %1\Cache\pc
 
rem Copy the executables and DLLs
copy c:\your\path\to\o3de-multiplayersample\build\windows_mono\bin\profile\*.* %1
 
rem Copy the AWSCore files
copy c:\your\path\to\o3de-multiplayersample\build\windows_mono\bin\profile\Gems\AWSCore\*.* %1\Gems\AWSCore
 
rem Copy the launch_client / launch_server files
copy c:\your\path\to\o3de-multiplayersample\launch_*.* %1
 
rem Copy the PIX DLLs if PIX is enabled in the build
:: copy c:\your\path\to\3rdParty\WinPixEventRuntime\bin\x64 %1
```

Note: The script above will copy everything from the profile directory. You can either remove files you don't require such as .pdb files to save space, or modify the script to copy just whats needed.

It's recommended during testing that you include source information for your build, such as the commit IDs for o3de, o3de-multiplayersample, and o3de-multiplayersample-assets.
You can add this as .txt file in the build folder.

Zip up the directory before running it to make sure the zip is "pure" without any logs or artifacts.

### Run the final build to verify it

```shell
MultiplayerSample.ServerLauncher.exe --console-command-file=launch_server.cfg --rhi=null -NullRenderer -bg_ConnectToAssetProcessor=0 -sys_PakPriority=2 -sv_terminateOnPlayerExit=false
MultiplayerSample.GameLauncher.exe --console-command-file=launch_client.cfg -bg_ConnectToAssetProcessor=0 -sys_PakPriority=2
```

After running, check the output logs to verify there aren't any crashes, missing assets, etc. If any assets are missing, go back to the AssetBundler step, add the assets, and repeat.

## Linux profile packaged builds

Instructions for Linux are similar to Windows instructions above. All examples are Ubuntu 22.04 which is the primary Linux platform for O3DE. See https://www.o3de.org/docs/welcome-guide/requirements/ for more details.

## Install Wwise
See instructions above but install Wwise for Linux Ubuntu

### Build profile build and process assets

Build and run MPS as per the [README_LINUX.md](../README_LINUX.md) and ensure all assets are built.