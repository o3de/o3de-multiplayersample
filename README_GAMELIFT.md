# MultiplayerSample Project with Amazon GameLift

This README covers testing and running MultiplayerSample with Amazon GameLift.

## Local testing with [GameLiftLocal](https://docs.aws.amazon.com/gamelift/latest/developerguide/integration-testing-local.html)

### Setup

1. Install the [AWS CLI](https://docs.aws.amazon.com/cli/latest/userguide/getting-started-install.html), if you don't already have it. You will need it to create a game session with GameLiftLocal.
1. Download Java Runtime Environment: https://www.java.com/en/download/manual.jsp
1. Download the [AmazonGameLift Server SDK](https://gamelift-release.s3-us-west-2.amazonaws.com/GameLift_06_03_2021.zip) and extract the "GameLiftLocal-1.0.5" directory somewhere easy to find.
1. Enable the "AWSGameLift" and "MPSGameLift" gem by adding them to MultiplayerSample/Gem/Code/enabled_gems.cmake
1. Build the server and game launchers for MultiplayerSample as normal, per [top-level README](/README.md).



### Testing

1. Open a command terminal from within the `GameLiftLocal` directory you downloaded before, and run it with the following command:
    ```sh
    java -jar GameLiftLocal.jar
    ```

1. From another terminal within the root of the MultiplayerSample project, start the server launcher (assumes the executable is the result of a `profile` build):
    ```sh
    ./build/windows/bin/profile/MultiplayerSample.ServerLauncher.exe --sv_gameLiftEnabled=true --sv_dedicated_host_onstartup=false --loadlevel=NewStarbase
    ```
Note: You may be inclined to move these cvars into a cfg file and start the game by passing the --console-command-file parameter, but don't. Some cvars are used during a system component Activate(), but the console-command-file is executed after all system components have been activated. (example: sv_gameLiftEnabled is used inside AWSGameLiftServerSystemComponent::Activate()).

1. Create a game session with the below command:
    ```sh
    aws gamelift create-game-session --endpoint-url http://localhost:8080 --maximum-player-session-count 2 --fleet-id fleet-123 --game-session-id hello-mps
    ```
    You should observe logs in the `GameLiftLocal` terminal which indicate it handled the create-game-session request, and see the `ServerLauncher` load its level.

1. Finally, start the game launcher with the follow command:
    ```sh
    ./build/windows/bin/profile/MultiplayerSample.GameLauncher.exe --MPSGameLiftClientSystemComponent.JoinSession hello-mps --cl_gameliftLocalEndpoint "http://localhost:8080"
    ```
1. The game launcher should be connected to the server and your player can run around.