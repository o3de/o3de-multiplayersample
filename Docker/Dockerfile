# Copyright (c) Contributors to the Open 3D Engine Project.
# For complete copyright and license terms please see the LICENSE at the root of this distribution.
#
# SPDX-License-Identifier: Apache-2.0 OR MIT
#

# The cpu architecture to base the docker base script from
ARG INPUT_ARCHITECTURE=amd64

# The root to base the docker script base from
ARG INPUT_IMAGE=ubuntu:22.04

# Argument to determining the package type ('server', 'headless-server', 'launcher', or 'unified-launcher')
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

FROM ${INPUT_ARCHITECTURE}/${INPUT_IMAGE}

# Argument to determining the package type ('server', 'headless-server', 'launcher', or 'unified-launcher')
ARG PACKAGE_TYPE=launcher  # Default to 'launcher'

# o3de repo arguments (redeclared)
ARG O3DE_REPO
ARG O3DE_BRANCH
ARG O3DE_COMMIT
ARG O3DE_EXTRAS_REPO
ARG O3DE_EXTRAS_BRANCH
ARG O3DE_EXTRAS_COMMIT
ARG O3DE_MPS_REPO
ARG O3DE_MPS_BRANCH
ARG O3DE_MPS_COMMIT
ARG O3DE_MPS_ASSETS_REPO
ARG O3DE_MPS_ASSETS_BRANCH
ARG O3DE_MPS_ASSETS_COMMIT

ENV WORKSPACE=/data/workspace

WORKDIR $WORKSPACE

ENV O3DE_REPO=${O3DE_REPO}
ENV O3DE_BRANCH=${O3DE_BRANCH}
ENV O3DE_COMMIT=${O3DE_COMMIT}

ENV O3DE_EXTRAS_REPO=${O3DE_EXTRAS_REPO}
ENV O3DE_EXTRAS_BRANCH=${O3DE_EXTRAS_BRANCH}
ENV O3DE_EXTRAS_COMMIT=${O3DE_EXTRAS_COMMIT}

ENV O3DE_MPS_REPO=${O3DE_MPS_REPO}
ENV O3DE_MPS_BRANCH=${O3DE_MPS_BRANCH}
ENV O3DE_MPS_COMMIT=${O3DE_MPS_COMMIT}

ENV O3DE_MPS_ASSETS_REPO=${O3DE_MPS_ASSETS_REPO}
ENV O3DE_MPS_ASSETS_BRANCH=${O3DE_MPS_ASSETS_BRANCH}
ENV O3DE_MPS_ASSETS_COMMIT=${O3DE_MPS_ASSETS_COMMIT}

ENV PACKAGE_TYPE=${PACKAGE_TYPE}

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
        libssl-dev libxcb-icccm4 \
        libxcb-image0 \
        libxcb-keysyms1 \
        libxcb-render-util0 \
        libxcb-randr0 \
        libnvidia-gl-470 \
        libunwind-dev \
        libzstd-dev \
        binutils-dev \
        libvulkan1 

COPY build.sh /data/workspace/build.sh

RUN /data/workspace/build.sh && \
    rm -rf ~/.o3de && \
    rm -rf ~/O3DE

ENV NVIDIA_VISIBLE_DEVICES all
ENV NVIDIA_DRIVER_CAPABILITIES all
 
ENTRYPOINT ["/bin/bash", "-c", "/data/workspace/launch.sh"]
