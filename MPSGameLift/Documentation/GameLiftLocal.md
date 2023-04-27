# MultiplayerSample Project with Amazon GameLift Local

This README covers testing and running MultiplayerSample with Amazon GameLift Local.

## Local testing with [GameLiftLocal](https://docs.aws.amazon.com/gamelift/latest/developerguide/integration-testing-local.html)

### Setup

1. Install the [AWS CLI](https://docs.aws.amazon.com/cli/latest/userguide/getting-started-install.html), if you don't already have it. You will need it to create a game session with GameLiftLocal.
1. Download Java Runtime Environment: https://www.java.com/en/download/manual.jsp
1. Download the [AmazonGameLift Server SDK](https://gamelift-release.s3-us-west-2.amazonaws.com/GameLift_06_03_2021.zip) and extract the "GameLiftLocal-1.0.5" directory somewhere easy to find.
1. Enable the "AWSGameLift" and "MPSGameLift" gem by adding them to MultiplayerSample/Gem/Code/enabled_gems.cmake
1. Build the server and game launchers for MultiplayerSample as normal, per [top-level README](/README.md).
1. Work in progress (WiP) step: Add your AWS region to Config/default_aws_resource_mappings.json (example: "Region": "us-west-2")
    a. Currently needed otherwise when the client initializes GameLift there will be an error about not having a region. This should be removed once we properly parse the game/player session which contains the fleet-id, region-id, etc  


### Testing

1. Open a command terminal from within the `GameLiftLocal` directory you downloaded before, and run it with the following command:
    ```sh
    java -jar GameLiftLocal.jar
    ```

1. From another terminal within the root of the MultiplayerSample project, start the server launcher (assumes the executable is the result of a `profile` build):
    ```sh
    ./build/windows/bin/profile/MultiplayerSample.ServerLauncher.exe --sv_gameLiftEnabled=true --sv_dedicated_host_onstartup=false --loadlevel=newstarbase
    ```
    sv_gameLiftEnabled: Causes the GameLift gem connect to GameLift on app startup
    sv_dedicated_host_onstartup: Stops the Multiplayer Gem from automatically hosting on startup and instead wait for GameLift to tell the server when it's okay to begin hosting (and which port to listen in on for connections) 

Note: You may be inclined to move these [cvars](https://www.o3de.org/docs/user-guide/appendix/cvars/) into a cfg file and start the server by passing the --console-command-file parameter, but don't. Some cvars are used during a system component Activate(), but the console-command-file is executed after all system components have been activated.
For example, sv_gameLiftEnabled is used inside AWSGameLiftServerSystemComponent::Activate().

1. Create a game session with the below command:
    ```sh
    aws gamelift create-game-session --endpoint-url http://localhost:8080 --maximum-player-session-count 2 --fleet-id fleet-123 --game-session-id hello-mps --game-properties Key=loadlevel,Value=NewStarbase
    ```

    You should observe logs in the `GameLiftLocal` terminal which indicate it handled the create-game-session request, and see the `ServerLauncher` load its level.

1. Finally, start the game launcher and connect to the server.

    Option 1: Command Line

    ```sh
    ./build/windows/bin/profile/MultiplayerSample.GameLauncher.exe --MPSGameLiftClientSystemComponent.JoinSession hello-mps --cl_gameliftLocalEndpoint "http://localhost:8080"
    ```

    Option 2: JSON Menu

    1. Launch game and load the GameLift connection menu level

        ```sh
        ./build/windows/bin/profile/MultiplayerSample.GameLauncher.exe --cl_gameliftLocalEndpoint "http://localhost:8080" --loadlevel="mpsgamelift/prefabs/GameLiftConnectJsonMenu.spawnable"
        ```

    1. Provide JSON and click 'Connect':

        ```json
        { "GameSessionId": "hello-mps", "PlayerId": "<any_unique_id>", "PlayerSessionId": "not_required_for_gamelift_local" }
        ```
1. The game launcher should be connected to the server and your player can run around.
