# O3DE MPS Project Export Testing Instructions

## Notes

* These testing instructions are for Windows and Linux
* Each step in the testing process will include commands for both Windows and Linux
* The following environment variables will be used for these instructions for consistency. Update the path values if needed to match the testing environment
  * Windows
    ```commandline
    set PROJECT_BASE=%HOMEPATH%\MPS_Test
    set PROJECT_EXPORT_BASE=%HOMEPATH%\MPS_Export
    ```
  * Linux
    ```bash
    export PROJECT_BASE=$HOME/MPS_Test
    export PROJECT_EXPORT_BASE=$HOME/MPS_Export
    ```

## Fresh Install and Setup


1. Clone the development branch of O3DE and configure it: <br/>
   <br/>
    **Windows**
    ```commandline
    git clone -b development https://github.com/o3de/o3de.git %PROJECT_BASE%\o3de
    %PROJECT_BASE%\o3de\python\get_python.bat
    %PROJECT_BASE%\o3de\scripts\o3de.bat register --this-engine
    ```
    **Linux**
    ```bash
    git clone -b development https://github.com/o3de/o3de.git %PROJECT_BASE%/o3de
    $PROJECT_BASE/o3de/python/get_python.sh
    $PROJECT_BASE/scripts/o3de.sh register --this-engine
    ```
2. Clone the development branch of the O3DE Multiplayer Sample Assets and its PopcornFX submodule <br/>
   <br/>
    **Windows**
    ```commandline
    git clone -b development https://github.com/o3de/o3de-multiplayersample-assets.git %PROJECT_BASE%\o3de-multiplayersample-assets
    git -C %PROJECT_BASE%\o3de-multiplayersample-assets submodule update --init --recursive
    ```
    
    **Linux**
    ```bash
    git clone -b development https://github.com/o3de/o3de-multiplayersample-assets.git %PROJECT_BASE%/o3de-multiplayersample-assets
    git -C $PROJECT_BASE/o3de-multiplayersample-assets submodule update --init --recursive
    ```
   > Note: You may need to check the git revision of the Popcorn FX Plugin under:
   > 
   >> ...\o3de-multiplayersample-assets\Gems\O3DEPopcornFXPlugin
   > 
   > to make sure it is the latest version.

   <br/>
3. Register the Gems path for the O3DE Multiplayer Sample Assets that is needed for the Multiplayer Sample<br/>
   <br/>
    **Windows**
    ```commandline
    %PROJECT_BASE%\o3de\scripts\o3de.bat register --all-gems-path %PROJECT_BASE%\o3de-multiplayersample-assets\Gems 
    ```
    **Linux**
    ```bash
    $PROJECT_BASE/o3de/scripts/o3de.sh register --all-gems-path $PROJECT_BASE/o3de-multiplayersample-assets/Gems 
    ```
   <br/>

4. Clone the development branch of the O3DE Multiplayer Sample and register the project:<br/><br/>

    **Windows**
    ```commandline
    git clone -b development https://github.com/o3de/o3de-multiplayersample.git %PROJECT_BASE%\o3de-multiplayersample
    %PROJECT_BASE%\o3de\scripts\o3de.bat register -pp %PROJECT_BASE%\o3de-multiplayersample
    ```
    **Linux**
    ```bash
    git clone -b development https://github.com/o3de/o3de-multiplayersample.git $PROJECT_BASE/o3de-multiplayersample
    $PROJECT_BASE/o3de/scripts/o3de.sh register -pp $PROJECT_BASE/o3de-multiplayersample
    ```

## Run Project Export
Generate the project packages using the project's `export_mps.py` script in conjunction with the O3DE export command.


1. Create the directory you want to output the game and server. <br/>
   <br/>
    **Windows**
    ```commandline
    mkdir %PROJECT_EXPORT_BASE%\packages
    ```
    **Linux**
    ```bash
    mkdir $PROJECT_EXPORT_BASE/packages
    ```
2. Run the o3de export script to generate the packages.<br/>
    <br/>
    **Windows**
    ```commandline
    %PROJECT_BASE%\o3de\scripts\o3de.bat export-project -pp %PROJECT_BASE%\o3de-multiplayersample -es ExportScripts\export_mps.py -out %PROJECT_EXPORT_BASE%\mps_server -cfg release -a zip -nounified -code -gl -assets -ll INFO -cba -- /m
    ```
    **Linux**
    ```commandline
    $PROJECT_BASE/o3de/scripts/o3de.sh export-project -pp $PROJECT_BASE/o3de-multiplayersample -es ExportScripts/export_mps.py -out $PROJECT_EXPORT_BASE/mps_server -cfg release -a zip -nounified -code -gl -assets -ll INFO
    ```
   
    > Note: The export commands described above will automatically enable the `MPSGameLift` gem when building the package. To opt-out of enabling GameLift, remove the `-gl` argument from the above command.
    
    > Note: The export commands above will also generate an archive (zip) of the output directories  
   
3. 

## Test Exported Project

1. Start testing MultiplayerSample by launching the server (You may need to provide admin privilege to enable a connection to AssetProcessor)

**Windows**
```commandline
%PROJECT_EXPORT_BASE%\packages\MultiplayerSampleServerPackage\MultiplayerSample.ServerLauncher.exe --rhi=null -NullRenderer --console-command-file=launch_server.cfg --net_udpDefaultTimeoutMs=20000
```

**Linux**
```bash
$PROJECT_EXPORT_BASE/packages/MultiplayerSampleServerPackage/MultiplayerSample.ServerLauncher --rhi=null -NullRenderer --console-command-file=launch_server.cfg --net_udpDefaultTimeoutMs=20000 &
```

2. Once the server is up and running, then launch the game 
`
 
**Windows**
```commandline
%PROJECT_EXPORT_BASE%\packages\MultiplayerSampleGamePackage\MultiplayerSample.GameLauncher.exe --connect=127.0.0.1 --net_udpDefaultTimeoutMs=20000
```

**Linux**
```bash
$PROJECT_EXPORT_BASE/packages/MultiplayerSampleGamePackage/MultiplayerSample.GameLauncher --connect=127.0.0.1 --net_udpDefaultTimeoutMs=20000
```


2. At this point, check to see if the game runs, and if you're able to run around, see particles, shoot, and hear sounds. Any errors or crashes should result in timestamped logs, which can be found at `%PROJECT_EXPORT_BASE%\packages\MultiplayerSampleServerPackage\user\log\server.log`, or `%PROJECT_EXPORT_BASE%\packages\MultiplayerSampleGamePackage\user\log\game.log`.
