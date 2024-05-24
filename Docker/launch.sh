#!/bin/bash
#
# Copyright (c) Contributors to the Open 3D Engine Project.
# For complete copyright and license terms please see the LICENSE at the root of this distribution.
#
# SPDX-License-Identifier: Apache-2.0 OR MIT
#

PROJECT_NAME=MultiplayerSample

if [ "$PACKAGE_TYPE" = "server" ]
then
    PACKAGE_FOLDER=$WORKSPACE/MPS_SERVER
    LAUNCHER_TARGET=ServerLauncher
elif [ "$PACKAGE_TYPE" = "headless-server" ]
then
    PACKAGE_FOLDER=$WORKSPACE/MPS_HEADLESS_SERVER
    LAUNCHER_TARGET=HeadlessServerLauncher
elif [ "$PACKAGE_TYPE" = "launcher" ]
then
    PACKAGE_FOLDER=$WORKSPACE/MPS_LAUNCHER
    LAUNCHER_TARGET=GameLauncher
elif [ "$PACKAGE_TYPE" = "unified-launcher" ]
then
    PACKAGE_FOLDER=$WORKSPACE/MPS_UNIFIED_LAUNCHER
    LAUNCHER_TARGET=UnifiedLauncher
else
    echo "Invalid package type: $PACKAGE_TYPE"
    exit 1
fi

cd $PACKAGE_FOLDER
./${PROJECT_NAME}.${LAUNCHER_TARGET} --bg_ConnectToAssetProcessor=0 -r_fullscreen=$RUN_FULLSCREEN ${@:1}

exit $?