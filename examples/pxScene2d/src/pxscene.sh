#!/bin/bash

#Get absolute path to this script
THIS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

cd $THIS_DIR

externalDir=../external

export LD_LIBRARY_PATH=$externalDir/png/.libs/:$externalDir/jpg/.libs/:$externalDir/curl/lib/.libs/:$externalDir/libnode/out/Release/obj.target

./pxscene $1 $2 $3 $4 $5 $6 $7

