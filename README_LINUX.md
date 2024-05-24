# MultiplayerSample Project for Linux

This README covers installation and running MultiplayerSample project on Ubuntu Linux.
Refer [Open 3D Engine on Linux](https://o3de.org/docs/user-guide/platforms/linux/) for setting up the engine on Linux.

## Prerequisites

This repository uses **Git LFS** to store large binary files. A GitHub personal access token is required to authenticate with the Git LFS service. You can setup your personal access token and credential manager with the following steps:

1. Create a Git Personal Access Token. Your personal access token credentials are required for authentication when you clone the repository. For more information, refer to [Create a personal access token with the \'repo\' scope.](https://docs.github.com/en/github/authenticating-to-github/creating-a-personal-access-token)

2. Verify you have a [credential manager installed and configured](https://github.com/microsoft/Git-Credential-Manager-Core). Recent versions of Git install a credential manager so that your credentials are stored and supplied automatically when required.

## Conventions used in these instructions

These instructions use the following installation paths. Be sure to substitute your local installation paths:

* O3DE installation root: `$HOME/o3de/`
* O3DE 3rd-party packages root: `$HOME/o3de-packages/`

## Step 1: Clone the repository

<span style="background-color:#4F3C3C">**NOTE:** You can clone the project to any local directory. If you clone the project inside an existing Git repository directory (for example, the directory that contains your local O3DE engine repository) you should add the o3de-multiplayersample project directory to the Git exclude file for the existing Git repository.</span>

### Option #1 (Recommended) - Cloning into a directory outside the engine repository directory

1. In a terminal, `cd` to the local directory where you'd like to clone the project, for example:

   ```shell
   mkdir $HOME/my-o3de-projects
   cd $HOME/my-o3de-projects
   ```

2. Clone the project.

   ```shell
   git clone https://github.com/o3de/o3de-multiplayersample.git
   Cloning into 'o3de-multiplayersample'...
   ```

3. Clone the assets. In this example the assets are cloned beside the muliplayersample project.

   ```shell
   git clone https://github.com/o3de/o3de-multiplayersample-assets.git
   Cloning into 'o3de-multiplayersample-assets'...
   ```

4. From inside your clone of o3de-multiplayersample-assets, update the submodules. This step adds some required content such as the PopcornFX Gem.

   ```shell
   cd o3de-multiplayersample-assets
   git submodule update --init --recursive
   ```
5. From your root project folder clone O3DE-Extras. The O3DE-Extras repo is required for the [WWISE Audio gem](https://github.com/o3de/o3de-extras/tree/development/Gems/AudioEngineWwise).
   ```shell
   cd $HOME/my-o3de-projects
   git clone https://github.com/o3de/o3de-extras/
   ```

### Option #2 - Cloning into the engine repository directory

1. Clone the project into a directory named 'o3de-multiplayersample' in your existing engine repository directory.

   ```shell
   git clone https://github.com/o3de/o3de-multiplayersample.git $HOME/o3de/o3de-multiplayersample
   Cloning into 'o3de-multiplayersample'...
   ```

1. Clone the asset Gems into a directory named 'o3de-multiplayersample-assets' in your existing engine Gems directory.

   ```shell
   git clone https://github.com/o3de/o3de-multiplayersample-assets.git $HOME/o3de/gems/o3de-multiplayersample-assets
   Cloning into 'o3de-multiplayersample-assets'...
   ```

1. From inside your clone of o3de-multiplayersample-assets, update the submodules. This step adds some required content such as the PopcornFX Gem.

   ```shell
   cd $HOME/o3de/gems/o3de-multiplayersample-assets
   git submodule update --init --recursive
   ```

1. Modify the local engine git exclude file to ignore the project directory.

   ```shell
   echo o3de-multiplayersample > $HOME/o3de/.git/info/exclude
   echo o3de-multiplayersample-assets > $HOME/o3de/.git/info/exclude
   ```

## Step 2. Register the engine, the project, and the Gems

<span style="background-color:#4F3C3C">**NOTE:** The following steps only need to be performed once.</span>

1. Register the engine.

   ```shell
   $HOME/o3de/scripts/o3de register --this-engine
   ```

1. Register the asset Gems.

   ```shell
   $HOME/o3de/scripts/o3de register --all-gems-path $HOME/my-o3de-projects/o3de-multiplayersample-assets/Gems
   ```
1. Register the audio gem.
   ```shell
   $HOME/o3de/scripts/o3de register --gem-path $HOME/my-o3de-projects/o3de-extras/Gems/AudioEngineWwise/
   ```
1. Register the project.

   ```shell
   $HOME/o3de/scripts/o3de register -p $HOME/my-o3de-projects/o3de-multiplayersample
   ```

The final step prints warnings that the compatibility check for MultiplayerSample and Blast will be skipped. These warnings can be ignored.

## Step 3. Configure and build

### Option #1 (Recommended) -  Project-centric approach

This option outputs all the project binaries in the project's build directory (for example `$HOME/my-o3de-projects/o3de-multiplayersample/build`). The following commands are run from the `o3de-multiplayersample` project directory.

1. Example project-centric configure command.

   ```shell
   cmake -B build/linux -S . -G "Ninja Multi-Config" -DLY_3RDPARTY_PATH=$HOME/o3de-packages
   ```

1. Example project-centric build command.

   ```shell
   cmake --build build/linux --target Editor MultiplayerSample.GameLauncher MultiplayerSample.ServerLauncher --config profile -j 8
   ```

### Option #2 - Engine-centric approach to building a project

This option will output all the project and engine binaries in the engine's build directory (for example, `$HOME/o3de/build`).

1. Example engine-centric configure command.

   ```shell
   cmake -S $HOME/o3de -B $HOME/o3de/build/linux -G "Ninja Multi-Config" -DLY_3RDPARTY_PATH=$HOME/o3de-packages -DLY_PROJECTS="$HOME/my-o3de-projects/o3de-multiplayersample"
   ```

1. Example engine-centric build command.

   ```shell
   cmake --build $HOME/o3de/build/linux --target Editor MultiplayerSample.GameLauncher MultiplayerSample.ServerLauncher --config profile -j 8
   ```

## Step 4. Setup the client and server

Under project root, there are two files: `launch_client.cfg` and `launch_server.cfg`.

1. `launch_client.cfg` contains the client connection setting. To connect to a server that is running locally, add the following line:

   ```shell
   connect
   ```

   To connect to a remote server, add the IP address of the server after the connect statement. For example:

   ```shell
   connect 192.168.0.20
   ```

2. `launch_server.cfg` contains the initial level to load:

   ```shell
   LoadLevel Levels/NewStarbase/NewStarbase.spawnable
   ```

## Step 5. Launch the server

<span style="background-color:#4F3C3C">**NOTE:** The commands in the following sections are run from the project root directory or the engine root directory depending on how the project was built in **Step 3**.</span>

### Option #1 - Launch the server with arguments

```shell
./build/linux/bin/profile/MultiplayerSample.ServerLauncher --console-command-file=launch_server.cfg 
```

Note that the `launch_server.cfg` configuration file is passed with the `--console-command-file` argument.

### Option #2 - Launch the server from a command file

Alternatively, you can run `launch_server.sh` which includes the `--console-command-file` argument.

```shell
./launch_server.sh
```

### Option #3 - Launch a headless server

If you do not need to see rendered output on your server, you can reduce resource usage by launching a headless server that uses the null renderer.

<span style="background-color:#4F3C3C">**NOTE:** Parameters to use null renderer must be passed on the command line as the console-command-file is parsed after rendering is configured.</span>

```shell
./build/linux/bin/profile/MultiplayerSample.ServerLauncher --console-command-file=launch_server.cfg -rhi=null -NullRenderer
```

## Step 6. Launch the Client

### Option #1 - Launch the client with arguments

The client launcher can be run with the following command:

```shell
./build/linux/bin/profile/MultiplayerSample.GameLauncher --console-command-file=launch_client.cfg
```

This command starts the client and connects to the server specified in `launch_client.cfg`.

### Option #2 - Launch the client from a command file

Alternatively, you can run `launch_client.cmd` (Windows) or `launch_client.sh` (Unix) which includes the `--console-command-file` argument.

```shell
./launch_client.cmd 
```

## License

For terms please see the LICENSE*.TXT files included in the root of this distribution.
