

```
xhost +local:docker
```


```
docker build --build-arg PACKAGE_TYPE=server -f Dockerfile -t amd64/o3de-mps-server:latest .
docker build --build-arg PACKAGE_TYPE=headless-server -f Dockerfile -t amd64/o3de-mps-headless-server:latest .
docker build --build-arg PACKAGE_TYPE=launcher -f Dockerfile -t amd64/o3de-mps-launcher:latest .
docker build --build-arg PACKAGE_TYPE=unified-launcher -f Dockerfile -t amd64/o3de-mps-unified-launcher:latest .
```

xhost +local:root

docker run --rm --network="bridge" --gpus all -e DISPLAY=:1 -v /tmp/.X11-unix:/tmp/.X11-unix -it  o3de-mps-server:latest -c /bin/bash
docker run --rm --network="bridge" --gpus all -e DISPLAY=:1 -v /tmp/.X11-unix:/tmp/.X11-unix -it  o3de-mps-headless-server:latest -c /bin/bash
docker run --rm --network="bridge" --gpus all -e DISPLAY=:1 -v /tmp/.X11-unix:/tmp/.X11-unix -it  o3de-mps-launcher:latest -c /bin/bash
docker run --rm --network="bridge" --gpus all -e DISPLAY=:1 -v /tmp/.X11-unix:/tmp/.X11-unix -it  o3de-mps:o3de-mps-unified-launcher -c /bin/bash




docker build --build-arg PACKAGE_TYPE=launcher -f Dockerfile -t amd64/o3de-mps-launcher:latest .

