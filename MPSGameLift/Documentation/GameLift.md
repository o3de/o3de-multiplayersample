# MultiplayerSample Project with Amazon GameLift

This README covers optional setup, testing and running on [Amazon GameLift](https://aws.amazon.com/gamelift/), an AWS service to make hosting and scaling game servers easier. It also provides guidance on how to test the Amazon GameLift integration on your local machine via [Amazon GameLift Anywhere](https://docs.aws.amazon.com/gamelift/latest/developerguide/fleets-creating-anywhere.html).

## Prepare the build for [Amazon GameLift](https://docs.aws.amazon.com/gamelift/index.html)

### Setup
1. Install the [AWS CLI](https://docs.aws.amazon.com/cli/latest/userguide/getting-started-install.html), if you don't already have it. You will need it to interact with Amazon GameLift.

    a. Confirm installation by running `aws --version`
    ```sh
    C:\> aws --version
    aws-cli/2.10.0 Python/3.11.2 Windows/10 exe/AMD64 prompt/off
    ```
    Even if you have already installed the AWS CLI, ensure it is updated as some commands may not be available on older versions.

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

    Make sure that the bootstrap.game.profile.setreg file has been added to one of the seed files. (also add debug if you want to support debug builds)

    ---

---
**NOTE**

This will not build the game client. If you need the client for testing, also run this command:
```sh
cmake --build build\windows_mono --target MultiplayerSample.GameLauncher --config profile -- /m /nologo
```

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


## Test your setup locally with GameLift Anywhere

Before you set up a real GameLift fleet, you can use GameLift Anywhere to quickly verify your build.
This guide mirrors the steps on the [Amazon GameLift Anywhere Developer Guide](https://docs.aws.amazon.com/gamelift/latest/developerguide/fleets-creating-anywhere.html)

### Create a Location

Create a location for your custom resources. This is a categorization akin to AWS regions.
When creating your custom location, the location name must start with `custom-`.

```sh
aws gamelift create-location --location-name custom-location-1 --region <Region>
```

If the operation was successful, the console will display the JSON result.
Record the `LocationName` for the next step. Example: custom-location-1

### Create a fleet

Creating an Anywhere fleet is a much faster process compared to creating a regular AWS fleet, which usually takes about an hour to setup.

```sh
aws gamelift create-fleet --name AnywhereFleet --compute-type ANYWHERE --locations Location=custom-location-1 --region <Region>
```

If the operation was successful, the console will display the JSON result.
Record the `FleetId` for the next steps. Example: **fleet-1a23bc4d-456e-78fg-h9i0-jk1l23456789**

### Register your local machine as a Compute

Register your local machine as a GameLift Anywhere Compute.
For ease of testing, we assume the Server and Client will be run on the same machine; so we can pass localhost (`127.0.0.1`) as the IP address.
If your machine is accessible via a public IP address, change that value as appropriate.

```sh
aws gamelift register-compute --compute-name CustomCompute1 --fleet-id <FleetId> --ip-address 127.0.0.1 --location custom-location-1 --region <Region>
```

If the operation was successful, the console will display the JSON result.
Record the `ComputeName` for the next steps. Example: **CustomCompute1**
Also record the `GameLiftServiceSdkEndpoint` for later. Example: **wss://us-west-2.api.amazongamelift.com**

### Get Compute auth token

```sh
aws gamelift get-compute-auth-token --fleet-id <FleedId> --compute-name <ComputeName>
```

If the operation was successful, the console will display the JSON result.
Record the `AuthToken` for the next steps. Example: **123a4b5c-d6e7-8fgh-9i01-2jklm34no567**

## Start an instance of the Game Server executable on your machine

To ensure GameLift recognizes your local machine as a Compute that is available to start game sessions, start a server locally with the appropriate credentials.

Notes:
- In the `HostId` property should be filled with the `ComputeName`;
- `ProcessId` can be omitted. A unique default `ProcessId` will be generated out of the timestamp.

```sh
C:\GameLiftPackageWindows\MultiplayerSample.ServerLauncher.exe --rhi=null -NullRenderer -bg_ConnectToAssetProcessor=0 --console-command-file=launch_server.cfg --sv_dedicated_host_onstartup=false --sv_gameLiftEnabled=true --sv_gameliftAnywhereEnabled=true --sv_gameliftAnywhereWebSocketUrl="<WebSocketUrl>" --sv_gameliftAnywhereAuthToken="<AuthToken>" --sv_gameliftAnywhereFleetId="<FleetId>" --sv_gameliftAnywhereHostId="<ComputeName>" --sv_gameliftAnywhereProcessId="<ProcessId>"
```

### Create a Game Session

```sh
aws gamelift create-game-session --region <Region> --location custom-location-1 --fleet-id <FleetId> --name GameSession1 --maximum-player-session-count 3
```

If the operation was successful, the console will display the JSON result.
Record the `GameSessionId` for the next steps. Example: **arn:aws:gamelift:us-west-2::gamesession/fleet-1a23bc4d-456e-78fg-h9i0-jk1l23456789/custom-location-1/gsess-ab1cd2ef-3gh4-5678-ijk9-0l1mn2o345p6**

If the operation fails, make sure the server is running. Ensure that `InitSDK` and `ProcessReady` calls were successful.

### Start Client

```sh
<path-to-multiplayer-sample>\build\windows_mono\bin\profile\MultiplayerSample.GameLauncher.exe -bg_ConnectToAssetProcessor=0 --loadlevel="mpsgamelift/prefabs/GameLiftConnectJsonMenu.spawnable"
```

Once started, the client should show a text area where the session information needs to be pasted into. You may need to press `~` on your keyboard to open the console and release the cursor from being bound to the client window.
It is recommended to prepare this JSON object in advance as the `PlayerSessionId` generated in the next step is only valid for 60 seconds.

```json
{ "GameSessionId": "<GameSessionId>", "PlayerId": "PlayerId", "PlayerSessionId": "<PlayerSessionId>" }
```

### Create a Player Session

Note: `PlayerId` from the JSON above and `--player-id` in the command below do not need to be the same

```sh
aws gamelift create-player-session --region <Region> --game-session-id <GameSessionId> --player-id Player1
```

If the operation was successful, the console will display the JSON result.
Record the `PlayerSessionId` for the next steps. Example: **psess-1a2b3c45-d6e7-89fg-0hij-12kl34m56no7**

### Connect the Client

Add the `PlayerSessionId` into the JSON and paste the JSON into the textarea inside the Client, then press "Connect".
The client should now successfully connect to your local server.


## Deploy to GameLift

### Upload the build to GameLift

---
**NOTE**

Builds are tied to Fleets; you may want to delete the existing build and fleet via the AWS Gamelift dashboard just so you don't accidentally reference old builds or old fleets in future steps.

---
 
```sh
aws gamelift upload-build --server-sdk-version 5.0.0 --operating-system WINDOWS_2016 --build-root C:\GameLiftPackageWindows\ --name MultiplayerSample --build-version v1.0 --region <Region>
```
Record BuildId for the next step. Example: **build-1a23bc4d-456e-78fg-h9i0-jk1l23456789**

### Create Fleet
After running this command it'll take about an hour for the fleet to activate. Check the status on the GameLift dashboard. 

```sh
aws gamelift create-fleet --region <Region> --name GameLiftO3DTest2016 --ec2-instance-type c5.large --fleet-type ON_DEMAND --build-id <BuildId> --runtime-configuration "GameSessionActivationTimeoutSeconds=300, ServerProcesses=[{LaunchPath=C:\game\MultiplayerSample.ServerLauncher.exe, Parameters= --rhi=null -sys_PakPriority=2 -NullRenderer -sv_terminateOnPlayerExit=true -bg_ConnectToAssetProcessor=0 --sv_gameLiftEnabled=true --sv_dedicated_host_onstartup=false --console-command-file=launch_server.cfg, ConcurrentExecutions=1}]" --ec2-inbound-permissions "FromPort=33450,ToPort=34449,IpRange=0.0.0.0/0,Protocol=UDP"
```
---
**NOTE**

The ec2-instance-type and fleet-type determines the kind of AWS resources used; your AWS account may incur costs.
https://aws.amazon.com/gamelift/pricing/

---

Record the FleetId for the next step. Example: **fleet-1a23bc4d-456e-78fg-h9i0-jk1l23456789**

### Create and Join Game Session
```sh
aws gamelift create-game-session --region <Region> --fleet-id <FleetId> --name foogamesession1 --maximum-player-session-count 10
```
Record GameSessionId for the next step. Example: **arn:aws:gamelift:us-west-2::gamesession/fleet-1a23bc4d-456e-78fg-h9i0-jk1l23456789/custom-location-1/gsess-ab1cd2ef-3gh4-5678-ijk9-0l1mn2o345p6**

Launch the game client with:
```sh
<path-to-multiplayer-sample>\build\windows_mono\bin\profile\MultiplayerSample.GameLauncher.exe -bg_ConnectToAssetProcessor=0 --loadlevel="mpsgamelift/prefabs/GameLiftConnectJsonMenu.spawnable"
```
```sh
aws gamelift create-player-session --region <Region> --game-session-id <GameSessionId> --player-id Player1
```
---
**NOTE**
PlayerId passed into create-player-session shouldn't be the same PlayerId passed into this JSON block; keep these unique. 
Record PlayerSessionId and use this in the game immediately because it expires after 60 seconds. Example: **psess-12345678-9012-3ab4-cd5e-67890f12gh34**

---

Paste in the game session and player session and click Connect. 
```json
{ "GameSessionId": "<GameSessionId>", "PlayerId": "player_id", "PlayerSessionId": "<PlayerSessionId>" }
```
