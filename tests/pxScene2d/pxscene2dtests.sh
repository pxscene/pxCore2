#!/bin/bash

#Get absolute path to this script
THIS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
machine=`uname -s`
cd $THIS_DIR

pxScene2dDir=../../examples/pxScene2d
pxScene2dSrc=$pxScene2dDir/src
if [ $machine = "Darwin" ];
then
pxCoreLibs=../../build/mac/
else
pxCoreLibs=../../build/glut/
fi

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

if [ $machine = "Darwin" ];
then
nodeLibs=$externalDir/libnode-v6.9.0/out/Release/
export DYLD_LIBRARY_PATH=$nodeLibs:$curlLibs:$pngLibs:$ftLibs:$zLibs:$pxCoreLibs
export LD_LIBRARY_PATH=$nodeLibs:$curlLibs:$pngLibs:$ftLibs:$zLibs:$pxCoreLibs
else
PathD=$externalLibs:$pxScene2dSrc:$externalDir/libnode-v6.9.0/out/Debug/obj.target
PathR=$externalLibs:$pxScene2dSrc:$externalDir/libnode-v6.9.0/out/Release/obj.target
export LD_LIBRARY_PATH=$PathR:$pxCoreLibs
fi
export NODE_PATH=$pxScene2dSrc

EXTERNALS_GLUT=false
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

#support for running js files from unittests
ln -s ../../examples/pxScene2d/src/shell.js shell.js
ln -s ../../examples/pxScene2d/src/init.js init.js
ln -s ../../examples/pxScene2d/src/node_modules/ node_modules
ln -s ../../examples/pxScene2d/src/rcvrcore/ rcvrcore
ln -s ../../examples/pxScene2d/src/FreeSans.ttf FreeSans.ttf
ln -s ../../examples/pxScene2d/src/package.json package.json

./pxscene2dtests

#remove temporary files created for running js files from unittests
rm -rf shell.js
rm -rf init.js
rm -rf node_modules
rm -rf rcvrcore
rm -rf FreeSans.ttf
rm -rf package.json

#gdb ./pxscene2dtests core

