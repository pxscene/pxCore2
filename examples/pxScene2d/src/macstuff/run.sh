#!/bin/bash
#Get absolute path to this script
THIS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

cd $THIS_DIR

export DYLD_LIBRARY_PATH=./
export LD_LIBRARY_PATH=./
./pxMain
exit 0