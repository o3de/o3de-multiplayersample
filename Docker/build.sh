#!/bin/bash
#
# Copyright (c) Contributors to the Open 3D Engine Project.
# For complete copyright and license terms please see the LICENSE at the root of this distribution.
#
# SPDX-License-Identifier: Apache-2.0 OR MIT
#


PROJECT_NAME=MultiplayerSample

BUILD_ROOT=$WORKSPACE/build

###############################################################################
# Clone and bootstrap O3DE
###############################################################################
git_clone_o3de() {
    echo "Cloning o3de from $O3DE_REPO/$O3DE_BRANCH into $O3DE_ROOT"
    git clone --single-branch -b $O3DE_BRANCH $O3DE_REPO $O3DE_ROOT && \
        git -C $O3DE_ROOT lfs install && \
        git -C $O3DE_ROOT lfs pull && \
        git -C $O3DE_ROOT reset --hard $O3DE_COMMIT 
    if [ $? -ne 0 ]
    then
        echo "Error cloning o3de from $O3DE_REPO"
        exit 1
    fi
}







if [ -d $O3DE_ROOT ]
then
    # O3DE root exists, keep track of this
    O3DE_CLONED=0
else
    # O3dE root does not exist, clone into the image
    git_clone_o3de()
    O3DE_CLONED=1
fi

$O3DE_ROOT/python/get_python.sh && \
    $O3DE_ROOT/scripts/o3de.sh register -ep $O3DE_ROOT
if [ $? -ne 0 ]
then
    echo "Error bootstrapping O3DE from $O3DE_REPO"
    exit 1
fi


###############################################################################
# Clone and register o3de-extras
###############################################################################
echo "Cloning o3de-extras from $O3DE_EXTRAS_REPO/$O3DE_EXTRAS_BRANCH into $O3DE_EXTRAS_ROOT"
git clone --single-branch -b $O3DE_EXTRAS_BRANCH $O3DE_EXTRAS_REPO $O3DE_EXTRAS_ROOT && \
    git -C $O3DE_EXTRAS_ROOT lfs install && \
    git -C $O3DE_EXTRAS_ROOT lfs pull && \
    git -C $O3DE_EXTRAS_ROOT reset --hard $O3DE_EXTRAS_COMMIT
if [ $? -ne 0 ]
then
    echo "Error cloning o3de-extras from $O3DE_EXTRAS_REPO"
    exit 1
fi

$O3DE_ROOT/scripts/o3de.sh register --all-gems-path $O3DE_EXTRAS_ROOT/Gems
if [ $? -ne 0 ]
then
    echo "Unable to register o3de-extras Gems"
    exit 1
fi


###############################################################################
# Clone and register o3de-multiplayersample-assets
###############################################################################
echo "Cloning o3de-multiplayersample-assets from $O3DE_MPS_ASSETS_REPO/$O3DE_MPS_ASSETS_BRANCH into $O3DE_MPS_ASSETS_ROOT"
git clone --single-branch -b $O3DE_MPS_ASSETS_BRANCH $O3DE_MPS_ASSETS_REPO $O3DE_MPS_ASSETS_ROOT && \
    git -C $O3DE_MPS_ASSETS_ROOT lfs install && \
    git -C $O3DE_MPS_ASSETS_ROOT lfs pull && \
    git -C $O3DE_MPS_ASSETS_ROOT reset --hard $O3DE_MPS_ASSETS_COMMIT && \
    git -C $O3DE_MPS_ASSETS_ROOT submodule update --init --recursive 
if [ $? -ne 0 ]
then
    echo "Unable to clone o3de-multiplayersample-assets from $O3DE_MPS_ASSETS_REPO"
    exit 1
fi

$O3DE_ROOT/scripts/o3de.sh register --all-gems-path $O3DE_MPS_ASSETS_ROOT/Gems
if [ $? -ne 0 ]
then
    echo "Unable to register o3de-multiplayersample-assets Gems"
    exit 1
fi


###############################################################################
# Clone and register o3de-multiplayersample
###############################################################################
echo "Cloning o3de-multiplayersample from $O3DE_MPS_REPO/$O3DE_MPS_BRANCH into $O3DE_MPS_ROOT"
git clone --single-branch -b $O3DE_MPS_BRANCH $O3DE_MPS_REPO $O3DE_MPS_ROOT && \
git -C $O3DE_MPS_ROOT lfs install && \
git -C $O3DE_MPS_ROOT lfs pull && \
git -C $O3DE_MPS_ROOT reset --hard $O3DE_MPS_COMMIT
if [ $? -ne 0 ]
then
    echo "Unable to clone o3de-multiplayersample from $O3DE_MPS_REPO"
    exit 1
fi

$O3DE_ROOT/scripts/o3de.sh register -pp $O3DE_MPS_ROOT
if [ $? -ne 0 ]
then
    echo "Unable to register the o3de-multiplayersample project"
    exit 1
fi


###############################################################################
# Track the git commits from all the repos
###############################################################################
echo -e "\n\
Repository                    | Commit \n\
------------------------------+-----------------------------------------\n\
o3de                          | $O3DE_REPO/tree/$(git -C $WORKSPACE/o3de rev-parse HEAD)\n\
o3de-extras                   | $O3DE_EXTRAS_REPO/tree/$(git -C $WORKSPACE/o3de-extras rev-parse HEAD) ) \n\
o3de-multiplayersample-assets | $O3DE_MPS_ASSETS_REPO/tree/$(git -C $WORKSPACE/o3de-multiplayersample-assets rev-parse HEAD) ) \n\
o3de-multiplayersample        | $O3DE_MPS_REPO/tree/$(git -C $WORKSPACE/o3de-multiplayersample rev-parse HEAD) ) \n\
\n\
" >> $WORKSPACE/git_commit.txt

###############################################################################
# Build O3DE tools for asset processing and asset bundling
###############################################################################

cmake -B $O3DE_MPS_ROOT/build/tools \
      -S $O3DE_MPS_ROOT \
      -G "Ninja Multi-Config" \
      -DLY_DISABLE_TEST_MODULES=ON \
      -DLY_STRIP_DEBUG_SYMBOLS=ON
if [ $? -ne 0 ]
then
    echo "Error generating O3DE tools projects"
    exit 1
fi

cmake --build $O3DE_MPS_ROOT/build/tools \
      --config profile \
      --target AssetProcessorBatch AssetBundlerBatch
if [ $? -ne 0 ]
then
    echo "Error building the O3DE tools projects"
    exit 1
fi

###############################################################################
# Build the assets for the Multiplayer Sample
###############################################################################
pushd $O3DE_MPS_ROOT/build/tools/bin/profile

# Initial run to process the assets
./AssetProcessorBatch

# Secondary run to re-process ones that missed dependencies
./AssetProcessorBatch

popd

###############################################################################
# Bundle the assets for ROSCon2023Demo
###############################################################################

WORKSPACE_BUNDLE_FOLDER=$WORKSPACE/bundles

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

mkdir -p $WORKSPACE_BUNDLE_FOLDER
mkdir -p $PACKAGE_FOLDER/Cache/linux


###############################################################################
# Generate the bundles
###############################################################################
pushd $O3DE_MPS_ROOT/build/tools/bin/profile
echo "Creating the game assetList ..."
./AssetBundlerBatch assetLists \
        --assetListFile $WORKSPACE_BUNDLE_FOLDER/game_linux.assetList \
        --platform linux \
        --project-path $O3DE_MPS_ROOT \
        --seedlistFile $O3DE_MPS_ROOT/AssetBundling/SeedLists/BasePopcornFxSeedList.seed \
        --seedlistFile $O3DE_MPS_ROOT/AssetBundling/SeedLists/GameSeedList.seed \
        --seedlistFile $O3DE_MPS_ROOT/AssetBundling/SeedLists/VFXSeedList.seed \
        --allowOverwrites
if [ $? -ne 0 ]
then
    echo "Error generating asset list from $WORKSPACE/RosConDemoSeedList.seed"
    exit 1
fi

echo "Creating the engine assetList ..."
./AssetBundlerBatch assetLists \
        --assetListFile $WORKSPACE_BUNDLE_FOLDER/engine_linux.assetList \
        --platform linux \
        --project-path $O3DE_MPS_ROOT \
        --addDefaultSeedListFiles \
        --allowOverwrites
if [ $? -ne 0 ]
then
    echo "Error generating default engine asset list"
    exit 1
fi

echo "Creating the game asset bundle (pak) ..."
./AssetBundlerBatch bundles \
        --platform linux \
        --project-path $O3DE_MPS_ROOT \
        --allowOverwrites \
        --assetListFile $WORKSPACE_BUNDLE_FOLDER/game_linux.assetList \
        --outputBundlePath $PACKAGE_FOLDER/Cache/linux/game_linux.pak
if [ $? -ne 0 ]
then
    echo "Error bundling generating game pak"
    exit 1
fi
             
echo "Creating the engine asset bundle (pak) ..."
./AssetBundlerBatch bundles \
        --platform linux \
        --project-path $O3DE_MPS_ROOT \
        --allowOverwrites \
        --assetListFile $WORKSPACE_BUNDLE_FOLDER/engine_linux.assetList \
        --outputBundlePath $PACKAGE_FOLDER/Cache/linux/engine_linux.pak
if [ $? -ne 0 ]
then
    echo "Error bundling generating engine pak"
    exit 1
fi

# Build the game launcher monolithically
echo "Building the ${PROJECT_NAME}.${LAUNCHER_TARGET}."
cmake -B $O3DE_MPS_ROOT/build/game \
      -S $O3DE_MPS_ROOT \
      -G "Ninja Multi-Config" \
      -DLY_DISABLE_TEST_MODULES=ON \
      -DLY_STRIP_DEBUG_SYMBOLS=ON \
      -DLY_MONOLITHIC_GAME=ON \
      -DALLOW_SETTINGS_REGISTRY_DEVELOPMENT_OVERRIDES=0 \
&& cmake --build $O3DE_MPS_ROOT/build/game \
         --config profile \
         --target ${PROJECT_NAME}.${LAUNCHER_TARGET}
if [ $? -ne 0 ]
then
    echo "Error bundling ${PROJECT_NAME}.${LAUNCHER_TARGET}"
    exit 1
fi

cp $O3DE_MPS_ROOT/build/game/bin/profile/${PROJECT_NAME}.${LAUNCHER_TARGET} $PACKAGE_FOLDER/ 
cp $O3DE_MPS_ROOT/build/game/bin/profile/*.json $PACKAGE_FOLDER/ 
cp $O3DE_MPS_ROOT/build/game/bin/profile/*.so $PACKAGE_FOLDER

echo Cleaning up data
rm -rf $WORKSPACE/o3de
rm -rf $WORKSPACE/o3de-extras
rm -rf $WORKSPACE/o3de-multiplayersample
rm -rf $WORKSPACE/o3de-multiplayersample-assets
rm -rf $HOME/.o3de
rm -rf $HOME/O3DE
rm -rf $WORKSPACE_BUNDLE_FOLDER

echo -e "Docker image built from the following repo information\n\n"
cat $WORKSPACE/git_commit.txt

exit 0
