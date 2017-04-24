#!/bin/bash

#Get absolute path to this script
THIS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

cd $THIS_DIR

pxScene2dDir=../../examples/pxScene2d
pxScene2dSrc=$pxScene2dDir/src
pxCoreLibs=../../build/glut/

externalDir=$pxScene2dDir/external

# Path to externals Libs...
pngLibs=$externalDir/png/.libs
jpgLibs=$externalDir/jpg/.libs
curlLibs=$externalDir/curl/lib/.libs
ftLibs=$externalDir/ft/objs/.libs
zLibs=$externalDir/zlib
jpegturboLibs=$externalDir/libjpeg-turbo/.libs/
westerosLibs=$externalDir/westeros/external/install/lib

# Aggregated Libs path
externalLibs=$pngLibs/:$jpgLibs/:$curlLibs/:$ftLibs/:$zLibs:$westerosLibs/:$jpegturboLibs/:rpc/

PathD=$externalLibs:$pxScene2dSrc:$externalDir/libnode-v6.9.0/out/Debug/obj.target
PathR=$externalLibs:$pxScene2dSrc:$externalDir/libnode-v6.9.0/out/Release/obj.target

export NODE_PATH=$pxScene2dSrc

export LD_LIBRARY_PATH=$PathR:$pxCoreLibs
EXTERNALS_GLUT=true
if [[ ! -z "$USE_EXTERNALS_GLUT" ]]
then
EXTERNALS_GLUT=$USE_EXTERNALS_GLUT
fi
if [ "$EXTERNALS_GLUT" ==  true ]
then
echo "using externals glut"
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$externalDir/freeglut/lib/
fi
export RT_LOG_LEVEL=warn

#echo $LD_LIBRARY_PATH

./pxscene2dtests

#gdb ./pxscene2dtests core

