#!/bin/sh

ulimit -c unlimited

pxSceneDir=../..
nodeBinaryDir=../../external/libnode/out/Release
jsbindingsDir=$pxSceneDir/src/jsbindings

echo "Start"
echo "   pxSceneDir=" $pxSceneDir
echo "nodeBinaryDir=" $nodeBinaryDir
echo "jsbindingsDir=" $jsbindingsDir
echo "\n"

export DYLD_LIBRARY_PATH=$pxSceneDir/external/png/.libs/:$pxSceneDir/external/curl/lib/.libs/:$pxSceneDir/external/ft/objs/.libs/
export LD_LIBRARY_PATH=$pxSceneDir/external/png/.libs/:$pxSceneDir/external/jpg/.libs/:$pxSceneDir/external/curl/lib/.libs/:$pxSceneDir/external/ft/objs/.libs/:$pxSceneDir/external/westeros/external/install/lib
export NODE_PATH=./:$jsbindingsDir/build/Debug:./node_modules

[ -f FontdinerSwanky.ttf ] || cp $pxSceneDir/src/FontdinerSwanky.ttf .
[ -f FreeSans.ttf ] || cp $pxSceneDir/src/FreeSans.ttf .

echo $LD_LIBRARY_PATH

node start.js url=$*
