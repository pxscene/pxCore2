#!/bin/bash
#Get absolute path to this script
THIS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

cd "$THIS_DIR"
source pxbenchmark.sh 1028 720 50 50
source pxbenchmark.sh 1920 1080 50 50
source pxbenchmark.sh 1028 720 500 500
source pxbenchmark.sh 1920 1080 500 500
