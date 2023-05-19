# MultiplayerSample Project with Amazon GameLift

This README covers optional setup, testing and running on [Amazon GameLift](https://aws.amazon.com/gamelift/), an AWS service to make hosting and scaling game servers easier. 

## Running with [Amazon GameLift](https://docs.aws.amazon.com/gamelift/index.html)

### Setup
1. Install the [AWS CLI](https://docs.aws.amazon.com/cli/latest/userguide/getting-started-install.html), if you don't already have it. You will need it to interact with Amazon GameLift.

    a. Confirm installation by running `aws --version`
    ```sh
    C:\> aws --version
    aws-cli/2.10.0 Python/3.11.2 Windows/10 exe/AMD64 prompt/off
    ```
1. Work in progress (WiP) step: Add your AWS region to Config/default_aws_resource_mappings.json (example: "Region": "us-west-2")

    a. Currently needed otherwise when the client initializes GameLift there will be an error about not having a region.

    b. This step will be removed once we properly parse the game-session data which contains the fleet-id, region-id, etc  

1. Use Export Project to Compile Code and Build Assets

    ```sh
    <path-to-o3de-engine>\scripts\o3de.bat export-project -es <path-to-multiplayer-sample>\MPSGameLift\Scripts\export_gamelift_server_package.py --code --assets -ll INFO
    ```
    A folder named "GameLiftWindowsServerPackage" containing the server will be created inside of the current working directory.

    ---
    **Important**

    The export_gamelift_server_package script only works for projects built using engine source, and won't work with engine as an sdk. 

    ---

    ---
    **Important**
    It's important to make sure that the bootstrap.game.profile.setreg file has been added to one of the seed files. (also add debug if you want to support debug builds)

    ---

1. Test the profile pak server and game locally
    Run the server in headless mode using `rhi=null` and `NullRenderer` parameters; the server appears as a white screen in headless mode.
    
    `C:\GameLiftPackageWindows\MultiplayerSample.ServerLauncher.exe --rhi=null -NullRenderer -bg_ConnectToAssetProcessor=0 -sys_PakPriority=2 --console-command-file=launch_server.cfg`
    
    `<path-to-multiplayer-sample>\build\windows\bin\profile\MultiplayerSample.GameLauncher.exe -bg_ConnectToAssetProcessor=0 --connect`

    ---
    **NOTE**

    Launch_server.cfg is required because there's a bug with multiplayer when calling --loadlevel in the command-line. See https://github.com/o3de/o3de/issues/15773.

    ---

1. Open C:\GameLiftPackageWindows\user\log\Server.log
    You should see the "New Starbase" level loaded
    ```
    Entities from new root spawnable 'levels/newstarbase/newstarbase.spawnable' are ready
    ```

## Prepare for GameLift
### Upload the build to GameLift

---
**NOTE**

Builds are tied to Fleets; you may want to delete the existing build and fleet via the AWS Gamelift dashboard just so you don't accidentally reference old builds or old fleets in future steps.

---
 
```sh
aws gamelift upload-build --operating-system WINDOWS_2016 --build-root C:\GameLiftPackageWindows\ --name MultiplayerSample --build-version v1.0 --region <Region>
```
Record BuildId for the next step. Example: build-fe4a2b2c-8ae5-4757-99a6-0ff372d03512

### Create Fleet
After running this command it'll take about an hour for the fleet to activate. Check the status on the GameLift dashboard. 

```sh
aws gamelift create-fleet --region <Region> --name GameLiftO3DTest2016 --ec2-instance-type c5.large --fleet-type ON_DEMAND --build-id <BuildId> --runtime-configuration "GameSessionActivationTimeoutSeconds=300, MaxConcurrentGameSessionActivations=2, ServerProcesses=[{LaunchPath=C:\game\MultiplayerSample.ServerLauncher.exe, Parameters= --rhi=null -sys_PakPriority=2 -NullRenderer -sv_terminateOnPlayerExit=true -bg_ConnectToAssetProcessor=0 --sv_gameLiftEnabled=true --sv_dedicated_host_onstartup=false --console-command-file=launch_server.cfg, ConcurrentExecutions=1}]" --ec2-inbound-permissions "FromPort=33450,ToPort=34449,IpRange=0.0.0.0/0,Protocol=UDP"
```
---
**NOTE**

The ec2-instance-type and fleet-type determines the kind of AWS resources used; your AWS account may incur costs.
https://aws.amazon.com/gamelift/pricing/

---

Record the FleetId for the next step. Example: fleet-1a49fc3e-892a-40fc-b2e9-aa7e11983182

### Create and Join Game Session
```sh
aws gamelift create-game-session --region <Region> --fleet-id <FleetId> --name foogamesession1 --maximum-player-session-count 10
```
Record GameSessionId for the next step. Example: arn:aws:gamelift:us-west-2::gamesession/fleet-1a49fc3e-892a-40fc-b2e9-aa7e11983182/gsess-4745e6ab-6cc0-44c7-b78d-acab9534f206

Launch the game client with:
```sh
./build/windows/bin/profile/MultiplayerSample.GameLauncher.exe --loadlevel="mpsgamelift/prefabs/GameLiftConnectJsonMenu.spawnable"
```
```sh
aws gamelift create-player-session --region <Region> --game-session-id <GameSessionId> --player-id Player1
```
---
**NOTE**
PlayerId passed into create-player-session shouldn't be the player id passed into these JSON block; keep these unique. 
Record PlayerSessionId and use this in the game immediately because it expires after 60 seconds. Example: psess-50311090-9283-4fb0-ad1a-94468e60fa16

---

Paste in the game session and player session and click Connect. 
```json
{ "GameSessionId": "<GameSessionId>", "PlayerId": "player_id", "PlayerSessionId": "<PlayerSessionId>" }
```
