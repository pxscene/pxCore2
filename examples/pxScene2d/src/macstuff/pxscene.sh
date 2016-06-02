#!/bin/bash

#Get absolute path to this script
THIS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

cd $THIS_DIR

export DYLD_LIBRARY_PATH=./lib/
export LD_LIBRARY_PATH=./lib/

./pxscene $1 $2 $3 $4 $5 $6 $7

