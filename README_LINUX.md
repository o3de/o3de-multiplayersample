# MultiplayerSample Project for Linux
## Download and Install

This README covers installation and running MultiplayerSample project on Ubuntu Linux.
Refer [Open 3D Engine on Linux](https://o3de.org/docs/user-guide/platforms/linux/) for setting up the engine on Linux.

This repository uses Git LFS for storing large binary files.  You will need to create a GitHub personal access token to authenticate with the LFS service.

## Setup
Follow the instructions in the [README](README.md) about general setup and the O3DE's [setup guide](https://www.o3de.org/docs/welcome-guide/setup/). 
Steps below require you to have either [O3DE installed](https://www.o3de.org/docs/welcome-guide/setup/installing-linux) or [built locally](https://www.o3de.org/docs/welcome-guide/setup/setup-from-github/building-linux/)

#### Option #1 (Recommended) - cloning into a folder outside the engine repository folder

```shell
# clone the project into a folder outside your engine repository folder
> git clone https://github.com/o3de/o3de-multiplayersample.git
Cloning into 'o3de-multiplayersample'...
```

```shell
# clone the assets into an external folder
> git clone https://github.com/o3de/o3de-multiplayersample-assets.git
Cloning into 'o3de-multiplayersample-assets'...
```

```shell
> cd o3de-multiplayersample-assets
# from inside your clone of o3de-multiplayersample-assets, update submodules:
# (PopcornFX gem might not be available otherwise)
> git submodule update --init --recursive
```

#### Option #2 - cloning into the engine repository folder

```shell
# clone the project into a folder named 'o3de-multiplayersample' in your existing engine repository folder
> git clone https://github.com/o3de/o3de-multiplayersample.git /path/to/o3de/o3de-multiplayersample
Cloning into 'o3de-multiplayersample'...

# clone the asset gems into a folder named 'o3de-multiplayersample-assets' in your existing engine gems folder
> git clone https://github.com/o3de/o3de-multiplayersample-assets.git /path/to/o3de/gems/o3de-multiplayersample-assets
Cloning into 'o3de-multiplayersample-assets'...

# from inside your clone of o3de-multiplayersample-assets, update submodules:
# (PopcornFX gem might not be available otherwise)
> git submodule update --init --recursive

# modify the local engine git exclude file to ignore the project folder
> echo o3de-multiplayersample >> /path/to/o3de/.git/info/exclude
> echo o3de-multiplayersample-assets >> /path/to/o3de/.git/info/exclude
```

If you have a Git credential helper configured, you should not be prompted for your credentials anymore.

### Step 2. Register the engine, the project, and the gems

```shell
# register the engine (only need to do this once)
> /path/to/o3de/scripts/o3de.sh register --this-engine

# register the asset gems (only need to do this once)
> /path/to/o3de/scripts/o3de.sh register --all-gems-path /path/to/o3de-multiplayersample-assets/Gems

# register the project (only need to do this once)
> /path/to/o3de/scripts/o3de.sh register -p /path/to/o3de-multiplayersample
```

The final step may print out warnings that the compatibility check for MultiplayerSample and Blast will be skipped. These warnings can be ignored.

### Step 3. Configure and build

#### Option #1 (Recommended) -  Project-centric approach

This option will output all the project binaries in the project's build folder e.g. c:/path/to/o3de-multiplayersample/build

```shell
# example configure command
> cmake -B build/linux -S . -G "Ninja Multi-Config" -DLY_3RDPARTY_PATH=$HOME/o3de-packages

# example build command
> cmake --build build/linux --target Editor MultiplayerSample.GameLauncher MultiplayerSample.ServerLauncher --config profile -j 8
```

#### Option #2 - Engine-centric approach to building a project

This option will output all the project and engine binaries in the engine's build folder e.g. /path/to/o3de/build

```shell
# example configure command
> cmake -S /path/to/o3de -B /path/to/o3de/build/linux -G "Ninja Multi-Config" -DLY_3RDPARTY_PATH=$HOME/o3de-packages -DLY_PROJECTS="/path/to/o3de-multiplayersample"

# example build command
> cmake --build /path/to/o3de/build/linux --target Editor MultiplayerSample.GameLauncher MultiplayerSample.ServerLauncher --config profile -j 8
```

### Step 4. Setup Client and Server

Under project root, there should be two files: `launch_client.cfg` and `launch_server.cfg`. 

File `launch_client.cfg` should contain:
```shell
connect
```

If connecting to a server not running on local host, add the IP after the connect statement, ie `connect 0.0.0.0`

File `launch_server.cfg` should contain the initial level to load:

```shell
LoadLevel Levels/NewStarbase/NewStarbase.spawnable
```

#### Running the Server

The server launcher can be run as follows

```shell
cd /path/to/o3de-multiplayersample/build
./bin/profile/MultiplayerSample.ServerLauncher --console-command-file=server.cfg
```

> Notice the launch_server.cfg is passed into the commandline. Any file passed into the console-command-file argument will be used when starting up the application.
For convenience, you can run launch_server.sh directly.
 
#### (Optional) Running the Server Headless

If you do not need to see rendered output on your servers, you can reduce resource usage by using the null renderer.

Note: Parameters to use null renderer must be passed on the commandline as the console-command-file is parsed after rendering is configured.

```shell
./bin/profile/MultiplayerSample.ServerLauncher --console-command-file=server.cfg
```

#### Running the Client
A client can be run with:

```shell
> cd /path/to/o3de-multiplayersample/build
> ./bin/profile/MultiplayerSample.GameLauncher --console-command-file=client.cfg
```

This will connect a client to the local server and start a multiplayer session.

## License

For terms please see the LICENSE*.TXT file at the root of this distribution.
