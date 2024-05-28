# Docker support for Multiplayer Sample

The O3DE Multiplayer Sample supports construction of Docker images on the Linux environment. 

## Prerequisites

* [Hardware requirements of o3de](https://www.o3de.org/docs/welcome-guide/requirements/)
* Any Linux distribution that supports Docker and the NVIDIA container toolkit (see below)
 * **Note** For the headless server flavor, the NVIDIA container toolkit is not required.
* At least 60 GB of free disk space
* Docker installed and configured
  * **Note** It is recommended to have Docker installed correctly and in a secure manner so that the Docker commands in this guide do not require elevated priviledges (sudo) in order to run them. See [Docker Engine post-installation steps](https://docs.docker.com/engine/install/linux-postinstall/) for more details.
* [NVidia container toolkit](https://docs.nvidia.com/datacenter/cloud-native/container-toolkit/install-guide.html#docker)

# Building the Docker image
The Docker scripts accepts arguments to control how to build the Docker image for the Multiplayer Sample. The main argument, `PACKAGE_TYPE` controls the type of launcher that will be built from the Docker build process. There are four different types of launchers supported:

1. **game** : The game client that will connect to a game server.
2. **server** : The game server that will accept remote game clients and will also provide a display of running game sessions. 
3. **unified** : The combined game client that also can run as the game server.
4. **headless** : The game server that will accept remote game clients but will not display and GUI or window. This is launcher is designed to run on headless servers.



| Argument                | Description                                                                | Default     
|-------------------------|----------------------------------------------------------------------------|-------------
| INPUT_IMAGE             | The base ubuntu docker image to base the build on                          | ubuntu:22.04
| INPUT_ARCHITECTURE      | The CPU architecture (amd64/aarch64). Will require QEMU if cross compiling | amd64
| O3DE_REPO               | The git repo for O3DE                                                      | https://github.com/o3de/o3de
| O3DE_BRANCH             | The branch for O3DE                                                        | development
| O3DE_COMMIT             | The commit on the branch for O3DE (or HEAD)                                | HEAD
| O3DE_EXTRAS_REPO        | The git repo for O3DE Extras                                               | https://github.com/o3de/o3de-extras
| O3DE_EXTRAS_BRANCH      | The branch for O3DE Extras                                                 | development
| O3DE_EXTRAS_COMMIT      | The commit on the branch for O3DE Extras (or HEAD)                         | HEAD
| O3DE_MPS_REPO           | The git repo for main Multiplayer Sample project                           | https://github.com/o3de/o3de-multiplayersample
| O3DE_MPS_BRANCH         | The branch for main Multiplayer Sample project                             | development
| O3DE_MPS_COMMIT         | The commit on the branch for Multiplayer Sample project (or HEAD)          | HEAD
| O3DE_MPS_ASSETS_REPO    | The git repo for Multiplayer Sample Assets                                 | https://github.com/o3de/o3de-multiplayersample-assets
| O3DE_MPS_ASSETS_BRANCH  | The branch for Multiplayer Sample Assets                                   | development
| O3DE_MPS_ASSETS_COMMIT  | The commit on the branch for Multiplayer Sample Assets (or HEAD)           | HEAD
| RUN_FULLSCREEN          | Option to launch the game, unified, or server in fullscreen mode (0=no, 1=yes) | 0


## Examples

### Locally (From the o3de-multiplayersample/Docker folder)

#### Game Launcher
```
docker build --build-arg PACKAGE_TYPE=game -f Dockerfile -t amd64/o3de-mps-game:latest .
```

### Unified Game Launcher
```
docker build --build-arg PACKAGE_TYPE=unified -f Dockerfile -t amd64/o3de-mps-unified:latest .
```

### Server Launcher
```
docker build --build-arg PACKAGE_TYPE=server -f Dockerfile -t amd64/o3de-mps-server:latest .
```

### Headless Server Launcher
```
docker build --build-arg PACKAGE_TYPE=headless -f Dockerfile -t amd64/o3de-mps-headless:latest .
```

### From github

#### Game Launcher

```
docker build --build-arg PACKAGE_TYPE=game -t amd64/o3de-mps-game:latest https://github.com/o3de/o3de-multiplayersample.git#development:Docker
```

### Unified Game Launcher
```
docker build --build-arg PACKAGE_TYPE=unified -t amd64/o3de-mps-unified:latest https://github.com/o3de/o3de-multiplayersample.git#development:Docker
```

### Server Launcher
```
docker build --build-arg PACKAGE_TYPE=server -t amd64/o3de-mps-server:latest https://github.com/o3de/o3de-multiplayersample.git#development:Docker
```

### Headless Server Launcher
```
docker build --build-arg PACKAGE_TYPE=headless -t amd64/o3de-mps-headless:latest https://github.com/o3de/o3de-multiplayersample.git#development:Docker
```


# Running the Docker image locally
Running the non-headless Docker images requires Vulkan and GPU acceleration provided by the NVIDIA drivers and container toolkit. The following directions will describe how to launch the Docker containers, utilizing the host Linux machine's X11 display and nvidia drivers, and connecting to the default 'bridge' network. (For advanced network isolation, refer to Docker's command-line reference for [network](https://docs.docker.com/reference/cli/docker/container/run/#network))

```
xhost +local:root
docker run --rm --gpus all -e DISPLAY=:1 --network="bridge" -v /tmp/.X11-unix:/tmp/.X11-unix -it $MPS_IMAGE
```
`MPS_IMAGE` is variable that can be one of the Docker image name that was used when building the image:
* **amd64/o3de-mps-game** : The game launcher
* **amd64/o3de-mps-unified** : The unified game/server launcher
* **amd64/o3de-mps-server** : The server launcher
* **amd64/o3de-mps-headless** : The headless server launcher (Does not require GPU acceleration)


# Deploying the Docker image
The Docker image can be published to any docker container registry and deployed on any Linux operating system that supports Docker.


