#!/bin/bash

#Get absolute path to this script
THIS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

cd $THIS_DIR

externalDir=../external

externalLibs=$externalDir/png/.libs/:$externalDir/jpg/.libs/:$externalDir/curl/lib/.libs/:$externalDir/ft/objs/.libs/:$externalDir/zlib:$externalDir/westeros/external/install/lib/:$externalDir/libjpeg-turbo/.libs/:rpc/

PathD=$externalLibs:$externalDir/libnode-v6.9.0/out/Debug/obj.target
PathR=$externalLibs:$externalDir/libnode-v6.9.0/out/Release/obj.target

export LD_LIBRARY_PATH=$PathR
export NODE_PATH=.

#export RT_LOG_LEVEL=info
 
./pxscene $1 $2 $3 $4 $5 $6 $7


# Development stuff...
#

#valgrind --tool=memcheck --leak-check=yes --show-reachable=yes --num-callers=20 --track-fds=yes ./pxscene $1 $2 $3 $4 $5 $6 $7
#./pxscene $1 $2 $3 $4 $5 $6 $7

#valgrind --tool=callgrind ./pxscene $1 $2

# NOTE: (rough) Process Performance Timing
#time ./pxscene  $1 $2 -R

# NOTE:  To use GDB ... use the DEBUG .so path for libnode
#gdb --args pxscene  $1 $2 $3 $4

#strace -o trace.txt pxscene $1 $2 $3 $4 $5 $6 $7
