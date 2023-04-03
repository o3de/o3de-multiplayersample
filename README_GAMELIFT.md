# MultiplayerSample Project with Amazon GameLift

This README covers testing and running MultiplayerSample with Amazon GameLift.

## Local testing with [GameLiftLocal](https://docs.aws.amazon.com/gamelift/latest/developerguide/integration-testing-local.html)

### Setup

1. Install the [AWS CLI](https://docs.aws.amazon.com/cli/latest/userguide/getting-started-install.html), if you don't already have it. You will need it to create a game session with GameLiftLocal.
1. Download the [AmazonGameLift Server SDK](https://gamelift-release.s3-us-west-2.amazonaws.com/GameLift_06_03_2021.zip) and extract the "GameLiftLocal-1.0.5" directory somewhere easy to find.
1. Build the server and game launchers for MultiplayerSample as normal, per [top-level README](/README.md).



### Testing

1. Open a command terminal from within the `GameLiftLocal` directory you downloaded before, and run it with the following command:
    ```sh
    java -jar GameLiftLocal.jar
    ```

2. From another terminal within the root of the MultiplayerSample project, start the server launcher (assumes the executable is the result of a `profile` build):
    ```sh
    ./build/bin/profile/MultiplayerSample.ServerLauncher.exe --console-command-file=launch_server.cfg
    ```

3. Create a game session with the below command:
    ```sh
    aws gamelift create-game-session --endpoint-url http://localhost:8080 --maximum-player-session-count 2 --fleet-id fleet-123 --game-session-id hello-mps
    ```
    You should observe logs in the `GameLiftLocal` terminal which indicate it handled the create-game-session request, and see the `ServerLauncher` load its level.

4. Finally, start the game launcher with the follow command:
    ```sh
    ./build/bin/profile/MultiplayerSample.GameLauncher.exe --console-command-file=launch_client_gameliftlocal.cfg --cl_gameliftLocalEndpoint "http://localhost:8080"
    ```