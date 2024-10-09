#!/bin/sh

# Place this script in the GameLift server package
# GameLift will run this script before opening the server
# https://docs.o3de.org/docs/user-guide/gems/reference/aws/aws-gamelift/build-packaging-for-linux/

sudo dnf install clang -y
sudo dnf install libunwind -y

echo 'Install Success'
