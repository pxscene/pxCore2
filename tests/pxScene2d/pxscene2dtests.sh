#!/bin/bash

#Get absolute path to this script
THIS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

cd $THIS_DIR

pxScene2dDir=../../examples/pxScene2d
pxScene2dSrc=$pxScene2dDir/src
externalDir=$pxScene2dDir/external

externalLibs=$externalDir/png/.libs/:$externalDir/jpg/.libs/:$externalDir/curl/lib/.libs/:$externalDir/ft/objs/.libs/:$externalDir/zlib:$externalDir/westeros/external/install/lib/:rpc/

PathD=$externalLibs:$pxScene2dSrc:$externalDir/libnode/out/Debug/obj.target
PathR=$externalLibs:$pxScene2dSrc:$externalDir/libnode/out/Release/obj.target

export LD_LIBRARY_PATH=$PathR

#echo $LD_LIBRARY_PATH

./pxscene2dtests

