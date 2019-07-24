#!/bin/bash

#Get absolute path to this script
THIS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

cd $THIS_DIR

externalDir=../external

externalLibs=$externalDir/openssl-1.0.2o/:$externalDir/png/.libs/:$externalDir/jpg/.libs/:$externalDir/curl/lib/.libs/:$externalDir/ft/objs/.libs/:$externalDir/zlib:$externalDir/westeros/external/install/lib/:$externalDir/libjpeg-turbo/.libs/:rpc/:$externalDir/gif/.libs/
PathD=$externalLibs:$externalDir/node/out/Debug/obj.target
PathR=$externalLibs:$externalDir/node/out/Release/obj.target

export LD_LIBRARY_PATH=$PathR

# TODO - come back and remove node_modules and externalDir before merging to master (once we don't need node_modules for webgl)
export NODE_PATH=.:./node_modules:$externalDir

#export RT_LOG_LEVEL=info

#valgrind integration
#suppressions are enabled to ignore the errors not interested
if [[ ! -z $ENABLE_VALGRIND ]] && [[ $ENABLE_VALGRIND -eq 1 ]]
then
if [ -z $VALGRINDLOGS ]
then
VALGRINDLOGS=valgrind_logs
fi
echo "valgrind --vgdb=yes --tool=memcheck --log-file=$VALGRINDLOGS --leak-check=yes --show-reachable=yes --num-callers=20 --track-fds=yes ./Spark --experimental-vm-modules $1 $2 $3 $4 $5 $6 $7"
if [ -z $SUPPRESSIONS ]
then
valgrind --vgdb=yes --tool=memcheck --log-file=$VALGRINDLOGS --leak-check=yes --show-reachable=yes --num-callers=20 --track-fds=yes ./Spark --experimental-vm-modules $1 $2 $3 $4 $5 $6 $7
else
valgrind --vgdb=yes --tool=memcheck --suppressions=$SUPPRESSIONS  --log-file=$VALGRINDLOGS --leak-check=yes --show-reachable=yes --num-callers=20 --track-fds=yes ./Spark --experimental-vm-modules $1 $2 $3 $4 $5 $6 $7
fi
else
./Spark --experimental-vm-modules $1 $2 $3 $4 $5 $6 $7
fi
#To run Spark as background process
#./Spark --experimental-vm-modules $1 $2 $3 $4 $5 $6 $7 < `tty` >> /var/tmp/spark.log 2>&1 &

# Development stuff...
#

#valgrind --tool=memcheck --leak-check=yes --show-reachable=yes --num-callers=20 --track-fds=yes ./Spark --experimental-vm-modules $1 $2 $3 $4 $5 $6 $7
#./Spark --experimental-vm-modules $1 $2 $3 $4 $5 $6 $7

#valgrind --tool=callgrind ./Spark --experimental-vm-modules $1 $2

# NOTE: (rough) Process Performance Timing
#time ./Spark  --experimental-vm-modules $1 $2 -R

# NOTE:  To use GDB ... use the DEBUG .so path for libnode
#gdb --args Spark --experimental-vm-modules $1 $2 $3 $4

#strace -o trace.txt Spark --experimental-vm-modules $1 $2 $3 $4 $5 $6 $7
