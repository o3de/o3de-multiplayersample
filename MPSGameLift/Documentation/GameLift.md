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

1. Use Export Project to Compile Code and Build Assets

    ```sh
    <path-to-o3de-engine>\scripts\o3de.bat export-project -es <path-to-multiplayer-sample>\MPSGameLift\Scripts\export_gamelift_server_package.py --code --assets -ll INFO
    ```
    A folder named "GameLiftPackageWindows" containing the server will be created inside of the current working directory.

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

The `--package-gamelauncher` command line option can be added to also package the game client.
A folder named "MultiplayerSampleGamePackage" containing the game launcher will be created inside of the current working directory.

---

1. Test the profile pak server and game locally without using GameLift
    Run the server in headless mode using `rhi=null` and `NullRenderer` parameters; the server appears as a white screen in headless mode.
    
    `.\GameLiftPackageWindows\MultiplayerSample.ServerLauncher.exe --rhi=null -NullRenderer --console-command-file=launch_server.cfg --net_udpDefaultTimeoutMs=20000`
    
    `.\MultiplayerSampleGamePackage\MultiplayerSample.GameLauncher.exe --connect=127.0.0.1 --net_udpDefaultTimeoutMs=20000`

    ---
    **NOTE**

    Launch_server.cfg is required because there's a bug with multiplayer when calling --loadlevel in the command-line. See https://github.com/o3de/o3de/issues/15773.
    net_udpDefaultTimeoutMs is increased to 20 seconds in case the initial client level load takes too long. (see https://github.com/o3de/o3de/issues/14659)
    ---

1. Open .\GameLiftPackageWindows\user\log\Server.log
    You should see a level load command. This is the "New Starbase" level.
    ```
    LoadLevel : <empty>
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
Also record the `GameLiftServiceSdkEndpoint` for passing into `sv_gameliftAnywhereWebSocketUrl` later. Example: **wss://us-west-2.api.amazongamelift.com**

### Get Compute auth token

```sh
aws gamelift get-compute-auth-token --fleet-id <FleetId> --compute-name <ComputeName>
```

If the operation was successful, the console will display the JSON result.
Record the `AuthToken` for the next steps. Example: **123a4b5c-d6e7-8fgh-9i01-2jklm34no567**
Note the `AuthToken` expiration timestamp (15 minutes). Be sure to use the `AuthToken` to connect before it expires. 

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
.\MultiplayerSampleGamePackage\MultiplayerSample.GameLauncher.exe
```

Once started, the client should show a text area where the session information needs to be pasted into. You may need to press `~` on your keyboard to open the console and release the cursor from being bound to the client window.

### Create a Player Session

Note: `PlayerId` from the JSON above and `--player-id` in the command below do not need to be the same

```sh
aws gamelift create-player-session --region <Region> --game-session-id <GameSessionId> --player-id Player1
```

If the operation was successful, the console will display the JSON result.
Record the `PlayerSessionId` for the next steps. Example: **psess-1a2b3c45-d6e7-89fg-0hij-12kl34m56no7**

### Connect the Client

Copy and paste the player session JSON table output into the textarea inside the Client, then press "Connect".
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

Create a fleet with your server build using the following command:

```sh
aws gamelift create-fleet --region <Region> --name GameLiftO3DTest --ec2-instance-type c5.large --fleet-type ON_DEMAND --build-id <BuildId> --runtime-configuration "GameSessionActivationTimeoutSeconds=300, ServerProcesses=[{LaunchPath=C:\game\MultiplayerSample.ServerLauncher.exe, Parameters= --rhi=null -sys_PakPriority=2 -NullRenderer -sv_terminateOnPlayerExit=true -bg_ConnectToAssetProcessor=0 --sv_gameLiftEnabled=true --sv_dedicated_host_onstartup=false --sv_gameSessionNoPlayerShutdownTimeoutSeconds=3600 --console-command-file=launch_server.cfg, ConcurrentExecutions=1}]" --ec2-inbound-permissions "FromPort=33450,ToPort=34449,IpRange=0.0.0.0/0,Protocol=UDP"
```

To run multiple servers on a single EC2 instance, you can define additional server processes as follows. Make sure the `--sv_port` parameter is set to a unique value for each process.

```sh
aws gamelift create-fleet --region <Region> --name GameLiftO3DTest2016 --ec2-instance-type c5.large --fleet-type ON_DEMAND --build-id <BuildId> --runtime-configuration "GameSessionActivationTimeoutSeconds=300, ServerProcesses=[{LaunchPath=C:\game\MultiplayerSample.ServerLauncher.exe, Parameters= --rhi=null -sys_PakPriority=2 -NullRenderer -sv_terminateOnPlayerExit=true -bg_ConnectToAssetProcessor=0 --sv_gameLiftEnabled=true --sv_dedicated_host_onstartup=false --sv_gameSessionNoPlayerShutdownTimeoutSeconds=3600 --sv_port=33460 --console-command-file=launch_server.cfg, ConcurrentExecutions=1}, {LaunchPath=C:\game\MultiplayerSample.ServerLauncher.exe, Parameters= --rhi=null -sys_PakPriority=2 -NullRenderer -sv_terminateOnPlayerExit=true -bg_ConnectToAssetProcessor=0 --sv_gameLiftEnabled=true --sv_dedicated_host_onstartup=false --sv_gameSessionNoPlayerShutdownTimeoutSeconds=3600 --sv_port=33465 --console-command-file=launch_server.cfg, ConcurrentExecutions=1}]" --ec2-inbound-permissions "FromPort=33450,ToPort=34449,IpRange=0.0.0.0/0,Protocol=UDP"
```

After running this command it'll take about an hour for the fleet to activate. Check the status on the GameLift dashboard. 

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
.\MultiplayerSampleGamePackage\MultiplayerSample.GameLauncher.exe
```
```sh
aws gamelift create-player-session --region <Region> --game-session-id <GameSessionId> --player-id Player1
```
---
**NOTE**
PlayerId passed into create-player-session shouldn't be the same PlayerId passed into this JSON block; keep these unique. 
Record PlayerSessionId and use this in the game immediately because it expires after 60 seconds. Example: **psess-12345678-9012-3ab4-cd5e-67890f12gh34**

---

Paste the player session JSON table output into the textbox and press "Connect".
For example,
```json
    {
        "PlayerSessionId": "psess-6a9a7352-8ee9-407f-ad06-cd09ba7c3ca2",
        "PlayerId": "Player1",
        "GameSessionId": "arn:aws:gamelift:us-west-2::gamesession/fleet-1b49cff7-eb2b-4f74-866a-959da3e9cf1f/custom-location-1/gsess-5850bac5-d4fb-4588-a489-c3b62bd5f099",
        "FleetId": "fleet-1b49cff7-eb2b-4f74-866a-959da3e9cf1f",
        "FleetArn": "arn:aws:gamelift:us-west-2:353687041169:fleet/fleet-1b49cff7-eb2b-4f74-866a-959da3e9cf1f",
        "CreationTime": "2023-06-08T14:32:12.811000-07:00",
        "Status": "RESERVED",
        "IpAddress": "127.0.0.1",
        "Port": 33450
    }
```
