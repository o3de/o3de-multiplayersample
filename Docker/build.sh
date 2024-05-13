#!/bin/bash

echo WORKSPACE=$WORKSPACE
echo PACKAGE TYPE=$PACKAGE_TYPE

cd $WORKSPACE

################################################
echo Cloning o3de..
git clone $O3DE_REPO --single-branch -b $O3DE_BRANCH -o o3de && \
git -C $WORKSPACE/o3de lfs install && \
git -C $WORKSPACE/o3de lfs pull && \
git -C $WORKSPACE/o3de reset --hard $O3DE_COMMIT
if [ $? -ne 0 ]
then
    echo "Unable to clone o3de"
    exit 1
fi


################################################
echo Fetching Python
$WORKSPACE/o3de/python/get_python.sh 
if [ $? -ne 0 ]
then
    echo "Unable to fetch python"
    exit 1
fi

################################################
echo Registering o3de
$WORKSPACE/o3de/scripts/o3de.sh register -ep $WORKSPACE/o3de
if [ $? -ne 0 ]
then
    echo "Unable to register o3de"
    exit 1
fi

################################################
echo Cloning o3de-extras
git clone $O3DE_EXTRAS_REPO --single-branch -b $O3DE_EXTRAS_BRANCH -o o3de-extras && \
git -C $WORKSPACE/o3de-extras reset --hard $O3DE_EXTRAS_COMMIT && \
git -C $WORKSPACE/o3de-extras lfs install && \
git -C $WORKSPACE/o3de-extras lfs pull 
if [ $? -ne 0 ]
then
    echo "Unable to clone o3de-extras"
    exit 1
fi



################################################
echo Registering o3de-extras Gems
$WORKSPACE/o3de/scripts/o3de.sh register --all-gems-path $WORKSPACE/o3de-extras/Gems
if [ $? -ne 0 ]
then
    echo "Unable to register o3de-extras Gems"
    exit 1
fi

################################################
echo Cloning o3de-multiplayersample-assets
git clone $O3DE_MPS_ASSETS_REPO --single-branch -b $O3DE_MPS_ASSETS_BRANCH -o o3de-multiplayersample-assets && \
git -C $WORKSPACE/o3de-multiplayersample-assets reset --hard $O3DE_MPS_ASSETS_COMMIT && \
git -C $WORKSPACE/o3de-multiplayersample-assets lfs install && \
git -C $WORKSPACE/o3de-multiplayersample-assets lfs pull && \
git -C $WORKSPACE/o3de-multiplayersample-assets submodule update --init --recursive 
if [ $? -ne 0 ]
then
    echo "Unable to clone o3de-multiplayersample-assets"
    exit 1
fi

################################################
echo Registering o3de-multiplayersample-assets Gems
$WORKSPACE/o3de/scripts/o3de.sh register --all-gems-path $WORKSPACE/o3de-multiplayersample-assets/Gems
if [ $? -ne 0 ]
then
    echo "Unable to register o3de-multiplayersample-assets Gems"
    exit 1
fi

################################################
echo Cloning o3de-multiplayersample
git clone $O3DE_MPS_REPO --single-branch -b $O3DE_MPS_BRANCH -o o3de-multiplayersample && \
git -C $WORKSPACE/o3de-multiplayersample reset --hard $O3DE_EXTRAS_COMMIT && \
git -C $WORKSPACE/o3de-multiplayersample lfs install && \
git -C $WORKSPACE/o3de-multiplayersample lfs pull
if [ $? -ne 0 ]
then
    echo "Unable to clone o3de-multiplayersample"
    exit 1
fi

################################################
echo Registering o3de-multiplayersample
$WORKSPACE/o3de/scripts/o3de.sh register -pp $WORKSPACE/o3de-multiplayersample
if [ $? -ne 0 ]
then
    echo "Unable to register o3de-multiplayersample-assets Gems"
    exit 1
fi

################################################
echo -e "\n\
Repository                    | Commit \n\
------------------------------+-----------------------------------------\n\
o3de                          | $O3DE_REPO/tree/$(git -C $WORKSPACE/o3de rev-parse HEAD)\n\
o3de-extras                   | $O3DE_EXTRAS_REPO/tree/$(git -C $WORKSPACE/o3de-extras rev-parse HEAD) ) \n\
o3de-multiplayersample-assets | $O3DE_MPS_ASSETS_REPO/tree/$(git -C $WORKSPACE/o3de-multiplayersample-assets rev-parse HEAD) ) \n\
o3de-multiplayersample        | $O3DE_MPS_REPO/tree/$(git -C $WORKSPACE/o3de-multiplayersample rev-parse HEAD) ) \n\
\n\
" >> $WORKSPACE/git_commit.txt

################################################
echo Creating package type $PACKAGE_TYPE
if [ "$PACKAGE_TYPE" = "server" ]
then
    # Create the 'server' package
    $WORKSPACE/o3de/scripts/o3de.sh export-project \
        -es $WORKSPACE/o3de//scripts/o3de/ExportScripts/export_source_built_project.py \
        -pp $WORKSPACE/o3de-multiplayersample \
        -ll DEBUG \
        --config profile \
        --tool-config profile \
        --build-tools \
        --seedlist $WORKSPACE/o3de-multiplayersample/AssetBundling/SeedLists/BasePopcornFxSeedList.seed \
        --seedlist $WORKSPACE/o3de-multiplayersample/AssetBundling/SeedLists/GameSeedList.seed \
        --seedlist $WORKSPACE/o3de-multiplayersample/AssetBundling/SeedLists/VFXSeedList.seed \
        --build-assets \
        --output-path $WORKSPACE/Export \
        -a none \
        --server-project-file-pattern-to-copy $WORKSPACE/o3de-multiplayersample/launch_server.cfg \
        -nogame \
        -nounified

    if [ $? -ne 0 ]
    then
        echo "Error creating '$PACKAGE_TYPE' package"
        exit 1
    fi

    PACKAGE_TARGET=$WORKSPACE/Export/MultiplayerSampleServerPackage#
    PACKAGE_LAUNCH_TARGET=MultiplayerSample.ServerLauncher
    ADDITIONAL_LAUNCH_ARG=--console-command-file=launch_server.cfg


elif [ "$PACKAGE_TYPE" = "headless-server" ]
then
    # Create the 'headless-server' package
    $WORKSPACE/o3de/scripts/o3de.sh export-project \
        -es $WORKSPACE/o3de//scripts/o3de/ExportScripts/export_source_built_project.py \
        -pp $WORKSPACE/o3de-multiplayersample \
        -ll DEBUG \
        --config profile \
        --tool-config profile \
        --build-tools \
        --seedlist $WORKSPACE/o3de-multiplayersample/AssetBundling/SeedLists/BasePopcornFxSeedList.seed \
        --seedlist $WORKSPACE/o3de-multiplayersample/AssetBundling/SeedLists/GameSeedList.seed \
        --seedlist $WORKSPACE/o3de-multiplayersample/AssetBundling/SeedLists/VFXSeedList.seed \
        --build-assets \
        --output-path $WORKSPACE/Export \
        -a none \
        --server-project-file-pattern-to-copy $WORKSPACE/o3de-multiplayersample/launch_server.cfg \
        -nogame \
        -noserver \
        --headless-server-launcher \
        -nounified

    if [ $? -ne 0 ]
    then
        echo "Error creating '$PACKAGE_TYPE' package"
        exit 1
    fi

    PACKAGE_TARGET=$WORKSPACE/Export/MultiplayerSampleHeadlessServerPackage#
    PACKAGE_LAUNCH_TARGET=MultiplayerSample.HeadlessServerLauncher
    ADDITIONAL_LAUNCH_ARG=--console-command-file=launch_server.cfg

elif [ "$PACKAGE_TYPE" = "launcher" ]
then
    # Create the 'launcher' package
    $WORKSPACE/o3de/scripts/o3de.sh export-project \
        -es $WORKSPACE/o3de//scripts/o3de/ExportScripts/export_source_built_project.py \
        -pp $WORKSPACE/o3de-multiplayersample \
        -ll DEBUG \
        --config profile \
        --tool-config profile \
        --build-tools \
        --seedlist $WORKSPACE/o3de-multiplayersample/AssetBundling/SeedLists/BasePopcornFxSeedList.seed \
        --seedlist $WORKSPACE/o3de-multiplayersample/AssetBundling/SeedLists/GameSeedList.seed \
        --seedlist $WORKSPACE/o3de-multiplayersample/AssetBundling/SeedLists/VFXSeedList.seed \
        --build-assets \
        --output-path $WORKSPACE/Export \
        -a none \
        --game-project-file-pattern-to-copy $WORKSPACE/o3de-multiplayersample/launch_client.cfg \
        -noserver \
        -nounified

    if [ $? -ne 0 ]
    then
        echo "Error creating '$PACKAGE_TYPE' package"
        exit 1
    fi

    PACKAGE_TARGET=$WORKSPACE/Export/MultiplayerSampleGamePackage
    PACKAGE_LAUNCH_TARGET=MultiplayerSample.GameLauncher

elif [ "$PACKAGE_TYPE" = "unified-launcher" ]
then
    # Create the 'unified-launcher' package
    $WORKSPACE/o3de/scripts/o3de.sh export-project \
        -es $WORKSPACE/o3de//scripts/o3de/ExportScripts/export_source_built_project.py \
        -pp $WORKSPACE/o3de-multiplayersample \
        -ll DEBUG \
        --config profile \
        --tool-config profile \
        --build-tools \
        --seedlist $WORKSPACE/o3de-multiplayersample/AssetBundling/SeedLists/BasePopcornFxSeedList.seed \
        --seedlist $WORKSPACE/o3de-multiplayersample/AssetBundling/SeedLists/GameSeedList.seed \
        --seedlist $WORKSPACE/o3de-multiplayersample/AssetBundling/SeedLists/VFXSeedList.seed \
        --build-assets \
        --output-path $WORKSPACE/Export \
        -a none \
        -noserver \
        -nogame

    if [ $? -ne 0 ]
    then
        echo "Error creating '$PACKAGE_TYPE' package"
        exit 1
    fi

    PACKAGE_TARGET=$WORKSPACE/Export/MultiplayerSampleUnifiedPackage
    PACKAGE_LAUNCH_TARGET=MultiplayerSample.UnifiedLauncher

else
    echo "Invalid package type $PACKAGE"
    exit 1
fi


echo -e "#!/bin/bash\n \
\n\
cat $WORKSPACE/git_commit.txt \n\
if [ "$1" = "console" ]\n\
then\n\
    cd $PACKAGE_TARGET\n\
    ./$PACKAGE_LAUNCH_TARGET --bg_ConnectToAssetProcessor=0 $ADDITIONAL_LAUNCH_ARG\n\
    exit $?\n\
else\n\
    /bin/bash\n\
    exit $?\n\
fi\n\
\n" > $WORKSPACE/launch.sh

chmod +x $WORKSPACE/launch.sh

echo Cleaning up data
rm -rf $WORKSPACE/o3de
rm -rf $WORKSPACE/o3de-extras
rm -rf $WORKSPACE/o3de-multiplayersample
rm -rf $WORKSPACE/o3de-multiplayersample-assets

echo -e "Docker image built from the following repo information\n\n"
cat $WORKSPACE/git_commit.txt

exit 0
