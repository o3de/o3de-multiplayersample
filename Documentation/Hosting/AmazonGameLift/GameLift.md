# MultiplayerSample Project with Amazon GameLift

This README covers testing and running MultiplayerSample with Amazon GameLift.

## Running with [GameLift](https://docs.aws.amazon.com/gamelift/index.html)

### Setup
1. Install the [AWS CLI](https://docs.aws.amazon.com/cli/latest/userguide/getting-started-install.html), if you don't already have it. You will need it to create a game session with GameLiftLocal.
1. Enable the "AWSGameLift" and "MPSGameLift" gem by adding them to MultiplayerSample/Gem/Code/enabled_gems.cmake
1. Build the server and game launchers for MultiplayerSample as normal, per [top-level README](/README.md).
1. Build AssetBundler.exe
    ```sh
    cmake --build build\windows --target Editor MultiplayerSample.GameLauncher MultiplayerSample.ServerLauncher AssetBundler --config profile -- /m /nologo
    ```
1. Build all the assets
    ```sh
    cmake --build build\windows --target MultiplayerSample.Assets --config profile -- /m /nologo
    ```
1. Work in progress (WiP) step: Add your AWS region to Config/default_aws_resource_mappings.json (example: "Region": "us-west-2")
    a. Currently needed otherwise when the client initializes GameLift there will be an error about not having a region. 
    b. This step will be removed once we properly parse the game-session data which contains the fleet-id, region-id, etc  

## Build Server for Windows (WiP)
1. WiP Step: Build a profile pak server/game build
    a. This step should be replaced by building a proper release build
1. Build Monolithic Server
    a. cmake -B build\windows_mono -S . -G "Visual Studio 16" -DLY_MONOLITHIC_GAME=1 -DALLOW_SETTINGS_REGISTRY_DEVELOPMENT_OVERRIDES=0
    b. cmake --build build\windows_mono --target MultiplayerSample.GameLauncher MultiplayerSample.ServerLauncher MultiplayerSample.UnifiedLauncher --config profile -- /m /nologo
1. Bundle Content
    a. Open .\build\windows\bin\profile\AssetBundler.exe
    b. Follow steps for "Create a bundle for game assets" and "Create a bundle for engine assets" and "Add bundles to the release game layout" here: https://www.o3de.org/docs/user-guide/packaging/asset-bundler/bundle-assets-for-release/

The "default seed lists" choice should choose all but 4 seed lists to make the engine_pc.pak
The other 4 seed lists should all get selected to make the game_pc.pak
It's important to make sure that the bootstrap.game.profile.setreg file has been added to one of the seed files. (also add debug if you want to support debug builds)
1. Create the Launcher Zip file
   Use the following .bat file or equivalent copy steps to create a directory with the launchers in it:
   ```sh
    rem Use this by calling 'make_release C:\GameLiftPackageWindows' to make a release directory
    mkdir %1
    mkdir %1\Cache
    mkdir %1\Cache\pc
    mkdir %1\Gems
    mkdir %1\Gems\AWSCore
    
    rem Copy the pak files
    copy .\AssetBundling\Bundles\*.pak %1\Cache\pc
    
    rem Copy the executables and DLLs
    copy .\build\windows_mono\bin\profile\*.* %1
    
    rem Copy the AWSCore files
    copy .\build\windows_mono\bin\profile\Gems\AWSCore\*.* %1\Gems\AWSCore
1. Test the profile pak server and game locally
    ```sh
    C:\GameLiftPackageWindows\MultiplayerSample.ServerLauncher.exe --rhi=null -NullRenderer -bg_ConnectToAssetProcessor=0 -sys_PakPriority=2 -sv_terminateOnPlayerExit=true --console-command-file=launch_server.cfg
    ```
    
    ```sh
    C:\GameLiftPackageWindows\MultiplayerSample.GameLauncher.exe -bg_ConnectToAssetProcessor=0 -sys_PakPriority=2 --connect
    ```

    Note: launch_server.cfg is required because there's a bug with multiplayer when calling --loadlevel in the command-line. See https://github.com/o3de/o3de/issues/15773.
1. Open C:\GameLiftPackageWindows\user\log\Server.log
    You should see the "New Starbase" level loaded
    ```
    Entities from new root spawnable 'levels/newstarbase/newstarbase.spawnable' are ready
    ```

## Prepare for GameLift
### Upload the build to GameLift
Note: Builds are tied to Fleets; you may want to delete the existing build and fleet via the AWS Gamelift dashboard just so you don't accidentally reference old builds or old fleets in future steps. 
`
aws gamelift upload-build --operating-system WINDOWS_2016 --build-root C:\GameLiftPackageWindows\ --name MultiplayerSample --build-version v1.0 --region us-west-2
`
Record BuildId for the next step. Example: build-fe4a2b2c-8ae5-4757-99a6-0ff372d03512

### Create Fleet
After running this command it'll take about an hour for the fleet to activate. Check the status on the GameLift dashboard. 

`
aws gamelift create-fleet --region us-west-2 --name GameLiftO3DTest2016 --ec2-instance-type c5.large --fleet-type ON_DEMAND --build-id <BuildId> --runtime-configuration "GameSessionActivationTimeoutSeconds=300, MaxConcurrentGameSessionActivations=2, ServerProcesses=[{LaunchPath=C:\game\MultiplayerSample.ServerLauncher.exe, Parameters= --rhi=null -sys_PakPriority=2 -NullRenderer -sv_terminateOnPlayerExit=true -bg_ConnectToAssetProcessor=0 --sv_gameLiftEnabled=true --sv_dedicated_host_onstartup=false --console-command-file=launch_server.cfg, ConcurrentExecutions=1}]" --ec2-inbound-permissions "FromPort=33450,ToPort=34449,IpRange=0.0.0.0/0,Protocol=UDP"
`

Record the FleetId for the next step. Example: fleet-1a49fc3e-892a-40fc-b2e9-aa7e11983182

### Create and Join Game Session
`
aws gamelift create-game-session --region us-west-2 --fleet-id <FleetId> --name foogamesession1 --maximum-player-session-count 3
`
Record GameSessionId for the next step. Example: arn:aws:gamelift:us-west-2::gamesession/fleet-1a49fc3e-892a-40fc-b2e9-aa7e11983182/gsess-4745e6ab-6cc0-44c7-b78d-acab9534f206

Launch the game client with:
```sh
    ./build/windows/bin/profile/MultiplayerSample.GameLauncher.exe --loadlevel="mpsgamelift/prefabs/GameLiftConnectJsonMenu.spawnable"
```
`
aws gamelift create-player-session --region us-west-2 --game-session-id <GameSessionId> --player-id Player1
`
Note: PlayerId passed into create-player-session shouldn't be the player id passed into these JSON block; keep these unique. 
Record PlayerSessionId and use this in the game immediately because it expires after 60 seconds. Example: psess-50311090-9283-4fb0-ad1a-94468e60fa16

Paste in the game session and player session and click Connect. 
`
{ "GameSessionId": "<GameSessionId>", "PlayerId": "player_id", "PlayerSessionId": "<PlayerSessionId>" }
`
