# MultiplayerSample Project with Amazon GameLift

This README covers testing and running MultiplayerSample with Amazon GameLift.

## Running with [GameLift](https://docs.aws.amazon.com/gamelift/index.html)

### Setup
1. Install the [AWS CLI](https://docs.aws.amazon.com/cli/latest/userguide/getting-started-install.html), if you don't already have it. You will need it to create a game session with GameLiftLocal.
1. Enable the "AWSGameLift" and "MPSGameLift" gem by adding them to MultiplayerSample/Gem/Code/enabled_gems.cmake
1. Build the server and game launchers for MultiplayerSample as normal, per [top-level README](/README.md).
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
    a. build\windows\bin\profile\AssetBundler.exe
    b. Follow steps for "Create a bundle for game assets" and "Create a bundle for engine assets" and "Add bundles to the release game layout" here: https://www.o3de.org/docs/user-guide/packaging/asset-bundler/bundle-assets-for-release/

The "default seed lists" choice should choose all but 4 seed lists to make the engine_pc.pak
The other 4 seed lists should all get selected to make the game_pc.pak
It's important to make sure that the bootstrap.game.profile.setreg file has been added to one of the seed files. (also add debug if you want to support debug builds)
1. Create the Launcher Zip file
   Use the following .bat file or equivalent copy steps to create a directory with the launchers in it:
   ```sh
    rem Use this by calling 'make_release D:\my\output\GameLiftPackageWindows' to make a release directory
    mkdir %1
    mkdir %1\Cache
    mkdir %1\Cache\pc
    mkdir %1\Gems
    mkdir %1\Gems\AWSCore
    
    rem Copy the pak files
    copy D:\github\o3de-multiplayersample\AssetBundling\Bundles\*.pak %1\Cache\pc
    
    rem Copy the executables and DLLs
    copy D:\github\o3de-multiplayersample\build\windows_mono\bin\profile\*.* %1
    
    rem Copy the AWSCore files
    copy D:\github\o3de-multiplayersample\build\windows_mono\bin\profile\Gems\AWSCore\*.* %1\Gems\AWSCore
    
    rem Copy the launch_client / launch_server files
    copy D:\github\o3de-multiplayersample\launch_*.* %1
    ```


## Prepare for GameLift
### Upload the build to GameLift
Note: Builds are tied to Fleets; you may want to delete the existing build and fleet via the AWS Gamelift dashboard just so you don't accidentally reference old builds or old fleets in future steps. 
`
aws gamelift upload-build --operating-system WINDOWS_2012 --build-root .\GameLiftPackageWindows\ --name MultiplayerSample --build-version v1.0 --region us-west-2
`
This returns the build id which will be used in the next steps. Example Build ID: build-f447776f-b4ba-4f1e-99cf-09d49f114942

### Create Fleet
After running this command it'll take about an hour for the fleet to activate. Check the status on the GameLift dashboard. 

`
aws gamelift create-fleet --region us-west-2 --name GlLyTest2F --ec2-instance-type c5.large --fleet-type ON_DEMAND --build-id build-f447776f-b4ba-4f1e-99cf-09d49f114942 --runtime-configuration "GameSessionActivationTimeoutSeconds=300, MaxConcurrentGameSessionActivations=2, ServerProcesses=[{LaunchPath=C:\game\MultiplayerSample.ServerLauncher.exe, Parameters= --sv_gameLiftEnabled=true --sv_dedicated_host_onstartup=false --loadlevel=NewStarbase, ConcurrentExecutions=10}]" --ec2-inbound-permissions "FromPort=33435,ToPort=33435,IpRange=0.0.0.0/0,Protocol=UDP" "FromPort=33235,ToPort=33235,IpRange=0.0.0.0/0,Protocol=UDP"
`

Record the fleet-id for the next step.

### Create and Join Game Session
`
aws gamelift create-game-session --region us-west-2 --fleet-id fleet-6ae4d52f-4a6b-4591-99da-122d37922490 --name foogamesession1 --maximum-player-session-count 3 --game-properties Key=sv_map,Value=MultiplayerSample
`

`
aws gamelift create-player-session --region us-west-2 --game-session-id arn:aws:gamelift:us-west-2::gamesession/fleet-6ae4d52f-4a6b-4591-99da-122d37922490/gsess-f2c14a63-e5fe-462f-9f8e-6be95b9dda26 --player-id Player1
`

Take the output of create-player-session and reformat the values to the custom session token like below, use this in the game immediately because it expires after 60 seconds.

Launch the game client with:
```sh
    ./build/windows/bin/profile/MultiplayerSample.GameLauncher.exe --MPSGameLiftClientSystemComponent.JoinSession hello-mps --cl_gameliftLocalEndpoint "http://localhost:8080"
```
Paste the custom player session token and click on Connect. Note: PlayerId passed into create-player-session shouldn't be the player id passed into the JSON block; keep these unique. 

`
{ "FleetId": "fleet-6ae4d52f-4a6b-4591-99da-122d37922490", "GameSessionId": "arn:aws:gamelift:us-west-2::gamesession/fleet-6ae4d52f-4a6b-4591-99da-122d37922490/gsess-f2c14a63-e5fe-462f-9f8e-6be95b9dda26", "IpAddress": "35.87.147.200", "PlayerId": "abc123", "PlayerSessionId": "psess-abbf0c36-14a6-4497-8441-1a89119d2bca" }
`

### Build the client exe and asset pak (AKA Client Release build)
1. Wip step: Build a profile pak build game launcher 
   a. The real instructions will be for  a proper a release build
1. WiP step: Open MultiplayerSample and add a new level called GameLiftConnectJsonMenu.
   a. Add a Ui Canvas Asset Ref which opens GameLiftConnectJson.uicanvas
   b. Toggle ON "Is Auto Loaded" 
   c. Add a loadlevel command to Registry/settings.multiplayersample_gamelauncher.setreg so that this level is loaded automatically
   Example:
    ```
    {
        "Amazon": {
            "AzCore": {
                "Runtime": {
                    "ConsoleCommands": {
                        "loadlevel": "GameLiftConnectJsonMenu"
                    }
                }
            }
        }
    }
    ```
    d. We'll eventually enable this level automatically 