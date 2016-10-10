#!/bin/bash

#Get absolute path to this script
THIS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

cd $THIS_DIR

pxScene2dDir=../../examples/pxScene2d
pxScene2dSrc=$pxScene2dDir/src

externalDir=$pxScene2dDir/external

# Path to externals Libs...
pngLibs=$externalDir/png/.libs
jpgLibs=$externalDir/jpg/.libs
curlLibs=$externalDir/curl/lib/.libs
ftLibs=$externalDir/ft/objs/.libs
zLibs=$externalDir/zlib
westerosLibs=$externalDir/westeros/external/install/lib

# Aggregated Libs path
externalLibs=$pngLibs/:$jpgLibs/:$curlLibs/:$ftLibs/:$zLibs:$westerosLibs/:rpc/

PathD=$externalLibs:$pxScene2dSrc:$externalDir/libnode/out/Debug/obj.target
PathR=$externalLibs:$pxScene2dSrc:$externalDir/libnode/out/Release/obj.target

export NODE_PATH=$pxScene2dSrc

export LD_LIBRARY_PATH=$PathR
export RT_LOG_LEVEL=warn

#echo $LD_LIBRARY_PATH

./pxscene2dtests

#gdb ./pxscene2dtests core

