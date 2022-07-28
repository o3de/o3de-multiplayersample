# MultiplayerSample Project

A simple third-person multiplayer sample for O3DE.

> **_NOTE:_** For Linux, see the Linux specific setup in [README_LINUX](./README_LINUX.md).

## Download and Install

This repository uses Git LFS for storing large binary files.  You will need to create a Github personal access token to authenticate with the LFS service.

### Create a Git Personal Access Token

You will need your personal access token credentials to authenticate when you clone the repository.

[Create a personal access token with the 'repo' scope.](https://docs.github.com/en/github/authenticating-to-github/creating-a-personal-access-token)

### (Recommended) Verify you have a credential manager installed to store your credentials

Recent versions of Git install a credential manager to store your credentials so you don't have to put in the credentials for every request.
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

#### Option #2 - cloning into the engine repository folder

```shell
# clone the project into a folder named 'o3de-multiplayersample' in your existing engine repository folder
> git clone https://github.com/o3de/o3de-multiplayersample.git c:/path/to/o3de/o3de-multiplayersample
Cloning into 'o3de-multiplayersample'...

# clone the asset gems into a folder named 'o3de-multiplayersample-assets' in your existing engine gems folder
> git clone https://github.com/o3de/o3de-multiplayersample-assets.git c:/path/to/o3de/gems/o3de-multiplayersample-assets
Cloning into 'o3de-multiplayersample-assets'...

# modify the local engine git exclude file to ignore the project folder
> echo o3de-multiplayersample > c:/path/to/o3de/.git/info/exclude
> echo o3de-multiplayersample-assets > c:/path/to/o3de/.git/info/exclude
```

If you have a Git credential helper configured, you should not be prompted for your credentials anymore.

### Step 2. Register the engine, the project, and the gems

```shell
# register the engine (only need to do this once)
> c:/path/to/o3de/scripts/o3de register --this-engine

# register the project (only need to do this once)
> c:/path/to/o3de/scripts/o3de register -p c:/path/to/o3de-multiplayersample

# register the asset gems (only need to do this once)
> c:/path/to/o3de/scripts/o3de register --all-gems-path c:/path/to/o3de-multiplayersample-assets/gems
```

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

Under project root, there should be 2 files: client.cfg and server.cfg. File client.cfg should contain:

```shell
connect
```

File server.cfg should contain:

```shell
host
LoadLevel Levels/SampleBase/SampleBase.spawnable
```

If these cfg files are not present, create them as they will be used to when launching server and client launchers.

#### Running the Server

A server can be run as follows

```shell
MultiplayerSample.ServerLauncher.exe --console-command-file=server.cfg 
```

#### (Optional) Running the Server Headless

If you do not need to see rendered output on your servers, you can reduce resource usage by using the null renderer.

Note: Parameters to use null renderer must be passed on the command line as the console-command-file is parsed after rendering is configured.

```shell
MultiplayerSample.ServerLauncher.exe --console-command-file=server.cfg -rhi=null -NullRenderer
```

#### Running the Server in the Editor

By default, launching a local server from the editor during Play Mode is enabled. To disable this behavior, update the `editorsv_enabled` value in the `editor.cfg` file to `false`.

Refer to the O3DE document [Test Multiplayer Games in the O3DE Editor](https://o3de.org/docs/user-guide/gems/reference/multiplayer/multiplayer-gem/test-in-editor/) for the complete list of console variables (cvar) which support play in the editor with servers.

#### Running the Client

A client can be run with:

```shell
MultiplayerSample.GameLauncher.exe --console-command-file=client.cfg
```

This will connect a client to the local server and start a multiplayer session.

## More Information

* [O3DE Networking](https://o3de.org/docs/user-guide/networking/)
* [Multiplayer Tutorials](https://o3de.org/docs/learning-guide/tutorials/multiplayer/)

## License

For terms please see the LICENSE*.TXT file at the root of this distribution.
