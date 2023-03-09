# MultiplayerSample Project

A third-person multiplayer game sample for O3DE. 

> **_NOTE:_** For Linux, see the Linux specific setup in [README_LINUX](./README_LINUX.md).

## Gameplay Overview
This game sample has players competing to hit the highest score to win. Played over a series of rounds, each round brings greater opportunity for rewards but at higher risks.  Players race to collect gems scattered all over the level.

Additionally, players have energy shields to protect them. Shooting other player damages their shields and getting hit damages your shields. Once your shields are depleted, you will respawn but at the cost of some of your collected gems.

Do you risk it all to win? 

Game features:
* 3rd-person character setup.
* Shooting with laser pistols.
* A configurable number of rounds; defaults to 3.
* Configurable gem spawning patterns per round to drive exploration.
* Support for 1 to 15 players (goal is to raise performance to support 25 players per level).
* Rich sounds and VFX support.
* Teleporters to aid exploration.
* Many points of extensibility.

Note: There's no penalty for respawn if you are knocked off the level or use the teleporters.

### Player Controls

* Move using: `w,a,s,d` 
* Sprint: Hold `shift`
* Jump: `space` 
* Look around: use mouse
* Fire primary weapon: `left mouse button`
* See scoreboard: Hold `tab`
* Open game menu: `esc`
* Draw/holster active weapon: `e`

To be implemented:
* Fire secondary weapon: `right mouse button`
* Crouch: `left ctrl` (not currently supported)
* Change Weapons: (not currently supported)



## Download and Install

This repository uses Git LFS for storing large binary files.  You will need to create a GitHub personal access token to authenticate with the LFS service.

### Create a Git Personal Access Token

You will need your personal access token credentials to authenticate when you clone the repository.

[Create a personal access token with the \'repo\' scope.](https://docs.github.com/en/github/authenticating-to-github/creating-a-personal-access-token)

### (Recommended) Verify you have a credential manager installed to store your credentials

Recent versions of Git install a credential manager to store your credentials, so you don't have to put in the credentials for every request.
It is highly recommended you check that you have a [credential manager installed and configured](https://github.com/microsoft/Git-Credential-Manager-Core)

### Step 1. Clone the repository

You can clone the project to any folder locally, including inside the engine folder. If you clone the project inside an existing Git repository (e.g. o3de) you should add the project folder to the Git exclude file for the existing repository.

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
> git clone https://github.com/o3de/o3de-multiplayersample.git c:/path/to/o3de/o3de-multiplayersample
Cloning into 'o3de-multiplayersample'...

# clone the asset gems into a folder named 'o3de-multiplayersample-assets' in your existing engine gems folder
> git clone https://github.com/o3de/o3de-multiplayersample-assets.git c:/path/to/o3de/gems/o3de-multiplayersample-assets
Cloning into 'o3de-multiplayersample-assets'...

# from inside your clone of o3de-multiplayersample-assets, update submodules:
# (PopcornFX gem might not be available otherwise)
> git submodule update --init --recursive

# modify the local engine git exclude file to ignore the project folder
> echo o3de-multiplayersample > c:/path/to/o3de/.git/info/exclude
> echo o3de-multiplayersample-assets > c:/path/to/o3de/.git/info/exclude
```

If you have a Git credential helper configured, you should not be prompted for your credentials anymore.

### Step 2. Register the engine, the project, and the gems

```shell
# register the engine (only need to do this once)
> c:/path/to/o3de/scripts/o3de register --this-engine

# register the asset gems (only need to do this once)
> c:/path/to/o3de/scripts/o3de register --all-gems-path c:/path/to/o3de-multiplayersample-assets/Gems

# register the project (only need to do this once)
> c:/path/to/o3de/scripts/o3de register -p c:/path/to/o3de-multiplayersample
```

The final step will print out warnings that the compatibility check for MultiplayerSample and Blast will be skipped. These warnings can be ignored.

### Step 3. Configure and build

#### Option #1 (Recommended) -  Project-centric approach

This option will output all the project binaries in the project's build folder e.g. c:/path/to/o3de-multiplayersample/build

```shell
# example configure command
> cmake -S c:/path/to/o3de-multiplayersample -B c:/path/to/o3de-multiplayersample/build/windows_vs2019 -G "Visual Studio 16" -DLY_3RDPARTY_PATH="c:/3rdparty"

# example build command
> cmake --build c:/path/to/o3de-multiplayersample/build/windows_vs2019 --target Editor MultiplayerSample.GameLauncher MultiplayerSample.ServerLauncher --config profile -- /m /nologo
```

#### Option #2 - Engine-centric approach to building a project

This option will output all the project and engine binaries in the engine's build folder e.g. c:/path/to/o3de/build

```shell
# example configure command
> cmake -S c:/path/to/o3de -B c:/path/to/o3de/build/windows_vs2019 -G "Visual Studio 16" -DLY_3RDPARTY_PATH="c:/3rdparty" -DLY_PROJECTS="c:/path/to/o3de-multiplayersample"

# example build command
> cmake --build c:/path/to/o3de/build/windows_vs2019 --target Editor MultiplayerSample.GameLauncher MultiplayerSample.ServerLauncher --config profile -- /m /nologo
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
MultiplayerSample.ServerLauncher.exe --console-command-file=launch_server.cfg 
```
Notice the launch_server.cfg is passed into the commandline. Any file passed into the console-command-file argument will be used when starting up the application.
For convenience, you can run launch_server.cmd (Windows) or launch_server.sh (Unix) directly. 

#### (Optional) Running the Server Headless

If you do not need to see rendered output on your servers, you can reduce resource usage by using the null renderer.

Note: Parameters to use null renderer must be passed on the command line as the console-command-file is parsed after rendering is configured.

```shell
MultiplayerSample.ServerLauncher.exe --console-command-file=launch_server.cfg -rhi=null -NullRenderer
```

#### Running the Server in the Editor

By default, launching a local server from the editor during Play Mode is enabled. To disable this behavior, update the `editorsv_enabled` value in the `editor.cfg` file to `false`.

Refer to the O3DE document [Test Multiplayer Games in the O3DE Editor](https://o3de.org/docs/user-guide/gems/reference/multiplayer/multiplayer-gem/test-in-editor/) for the complete list of console variables (cvar) which support play in the editor with servers.

#### Running the Client

The client launcher can be run with:

```shell
MultiplayerSample.GameLauncher.exe --console-command-file=launch_client.cfg
```

This will connect a client to the local server and start a multiplayer session.
For convenience, you can run launch_client.cmd (Windows) or launch_client.sh (Unix) directly.

#### Debugging in Visual Studio
When debugging MultiplayerSample.GameLauncher and MultiplayerSample.ServerLauncher from Visual Studio it's helpful to automatically host and connect; thereby avoiding having to open the console (~) once the application opens and explicitly executing the `host` and `loadlevel` command on server, or the `connect` command on client. For convenience, Gem/Code/CMakeLists.txt defines `ADDITIONAL_VS_DEBUGGER_COMMAND_ARGUMENTS` which allows Visual Studio to automatically populate the debugger with command arguments. 

By default, launch_client.cfg is used when debugging the GameLauncher and launch_server.cfg is used when debugging the ServerLauncher.

> When debugging set `net_UdpTimeoutConnections` to false, this will prevent connection closures when stopped on breakpoints.

## Levels in this Project
This project ships with several levels, the ones of note are:

1. NewStarBase - The main game level, default to using this level
2. GamePlayTest - Everything needed for gameplay, but in a tiny, fast loading level. Gems, HUD, everything is there.
3. StartMenu - An example menu to enable joining, hosting and connecting to servers.
4. MultiplayerScriptingSample - An example of Multiplayer and scripting.

Other levels in the project are there for testing or performance evaluation purposes and can be ignored.

## More Information
* [O3DE Networking](https://o3de.org/docs/user-guide/networking/)
* [Multiplayer Tutorials](https://o3de.org/docs/learning-guide/tutorials/multiplayer/)
* [Networking/Multiplayer Settings](https://www.o3de.org/docs/user-guide/networking/settings/)

## License

For terms please see the LICENSE*.TXT file at the root of this distribution.
