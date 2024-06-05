# Copyright (c) Contributors to the Open 3D Engine Project.
# For complete copyright and license terms please see the LICENSE at the root of this distribution.
#
# SPDX-License-Identifier: Apache-2.0 OR MIT
#

ARG INPUT_ARCHITECTURE=amd64

ARG INPUT_IMAGE=ubuntu

ARG INPUT_TAG=jammy

FROM ${INPUT_ARCHITECTURE}/${INPUT_IMAGE}:${INPUT_TAG}

# Argument for determining the package type ('server', 'headless-server', 'game', or 'unified-launcher')
ARG PACKAGE_TYPE=launcher  # Default to 'launcher'

# o3de repo arguments
ARG O3DE_REPO=https://github.com/o3de/o3de
ARG O3DE_BRANCH=development
ARG O3DE_COMMIT=HEAD

# o3de-extras repo arguments
ARG O3DE_EXTRAS_REPO=https://github.com/o3de/o3de-extras
ARG O3DE_EXTRAS_BRANCH=development
ARG O3DE_EXTRAS_COMMIT=HEAD

# o3de-multiplayersample repo arguments
ARG O3DE_MPS_REPO=https://github.com/o3de/o3de-multiplayersample
ARG O3DE_MPS_BRANCH=development
ARG O3DE_MPS_COMMIT=HEAD

# o3de-multiplayersample-assets repo arguments
ARG O3DE_MPS_ASSETS_REPO=https://github.com/o3de/o3de-multiplayersample-assets
ARG O3DE_MPS_ASSETS_BRANCH=development
ARG O3DE_MPS_ASSETS_COMMIT=HEAD

# Argument to determining the package type ('server', 'headless', 'launcher', or 'unified')
ARG PACKAGE_TYPE=launcher  # Default to 'launcher'

# Argument to run fullscreen (for game, unified, and server package types)
ARG RUN_FULLSCREEN=0

# Set the workspace to work from
ENV WORKSPACE=/data/workspace
WORKDIR $WORKSPACE

# o3de Environment
ENV O3DE_REPO=$O3DE_REPO
ENV O3DE_BRANCH=$O3DE_BRANCH
ENV O3DE_COMMIT=$O3DE_COMMIT
ENV O3DE_ROOT=$WORKSPACE/o3de

# o3de-extras Environment
ENV O3DE_EXTRAS_REPO=$O3DE_EXTRAS_REPO
ENV O3DE_EXTRAS_BRANCH=$O3DE_EXTRAS_BRANCH
ENV O3DE_EXTRAS_COMMIT=$O3DE_EXTRAS_COMMIT
ENV O3DE_EXTRAS_ROOT=$WORKSPACE/o3de-extras

# o3de-multiplayersample Environment
ENV O3DE_MPS_ASSETS_REPO=$O3DE_MPS_ASSETS_REPO
ENV O3DE_MPS_ASSETS_BRANCH=$O3DE_MPS_ASSETS_BRANCH
ENV O3DE_MPS_ASSETS_COMMIT=$O3DE_MPS_ASSETS_COMMIT
ENV O3DE_MPS_ASSETS_ROOT=$WORKSPACE/o3de-multiplayersample-assets

# o3de-multiplayersample-assets Environment
ENV O3DE_MPS_REPO=$O3DE_MPS_REPO
ENV O3DE_MPS_BRANCH=$O3DE_MPS_BRANCH
ENV O3DE_MPS_COMMIT=$O3DE_MPS_COMMIT
ENV O3DE_MPS_ROOT=$WORKSPACE/o3de-multiplayersample

# Validate and set the package type
RUN if [ $PACKAGE_TYPE = "game" ];then echo "Building Game Launcher Image"; \
    elif [ $PACKAGE_TYPE = "unified" ];then echo "Building Unified Launcher Image"; \
    elif [ $PACKAGE_TYPE = "server" ];then echo "Building Server Launcher Image"; \
    elif [ $PACKAGE_TYPE = "headless" ];then echo "Building Headless Server Launcher Image"; \
    else echo "Invalid PACKAGE_TYPE argument '$PACKAGE_TYPE'. Must be one of ('server', 'headless', 'game', or 'unified')" && exit 1; \
    fi
ENV PACKAGE_TYPE=$PACKAGE_TYPE
ENV RUN_FULLSCREEN=$RUN_FULLSCREEN

# Add additional package repositories needed for packages
RUN apt-get update && \
    apt-get upgrade -y && \
    apt-get install -y --no-install-recommends \
        ca-certificates \
        git \
        git-lfs \
        libstdc++-12-dev \
        clang\
        ninja-build \
        cmake \
        libglu1-mesa-dev \
        libxcb-xinerama0 \
        libxcb-xinput0 \
        libxcb-xinput-dev \
        libxcb-xfixes0-dev \
        libxcb-xkb-dev \
        libxkbcommon-dev \
        libxkbcommon-x11-dev \
        libfontconfig1-dev \
        libcurl4-openssl-dev \
        libsdl2-dev \
        zlib1g-dev \
        mesa-common-dev \
        libssl-dev \
        libxcb-icccm4 \
        libxcb-image0 \
        libxcb-keysyms1 \
        libxcb-render-util0 \
        libxcb-randr0 \
        libnvidia-gl-470 \
        libunwind-dev \
        libzstd-dev \
        binutils-dev \
        libvulkan1 

COPY build.sh $WORKSPACE/build.sh
COPY launch.sh $WORKSPACE/launch.sh

RUN $WORKSPACE/build.sh \
    && rm $WORKSPACE/build.sh

ENV NVIDIA_VISIBLE_DEVICES all
ENV NVIDIA_DRIVER_CAPABILITIES all
 
ENTRYPOINT ["/bin/bash", "-c", "/data/workspace/launch.sh"]
