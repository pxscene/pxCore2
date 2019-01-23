#!/bin/bash

#Get absolute path to this script
SOURCE="${BASH_SOURCE[0]}"
while [ -h "$SOURCE" ]; do # resolve $SOURCE until the file is no longer a symlink
  THIS_DIR="$( cd -P "$( dirname "$SOURCE" )" >/dev/null 2>&1 && pwd )"
  SOURCE="$(readlink "$SOURCE")"
  [[ $SOURCE != /* ]] && SOURCE="$THIS_DIR/$SOURCE" # if $SOURCE was a relative symlink, we need to resolve it relative to the path where the symlink file was located
done
THIS_DIR="$( cd -P "$( dirname "$SOURCE" )" >/dev/null 2>&1 && pwd )"

cd $THIS_DIR

externalDir=../external
rtRemoteDir=../../../remote

externalLibs=$externalDir/png/.libs/:$externalDir/jpg/.libs/:$externalDir/curl/lib/.libs/:$externalDir/ft/objs/.libs/:$externalDir/zlib:$externalDir/westeros/external/install/lib/:$externalDir/libjpeg-turbo/.libs/:rpc/:$rtRemoteDir/

PathD=$externalLibs:$externalDir/libnode-v6.9.0/out/Debug/obj.target
PathR=$externalLibs:$externalDir/libnode-v6.9.0/out/Release/obj.target

export LD_LIBRARY_PATH=$PathR

export NODE_PATH=.

#export RT_LOG_LEVEL=info

#valgrind integration
#suppressions are enabled to ignore the errors not interested
if [[ ! -z $ENABLE_VALGRIND ]] && [[ $ENABLE_VALGRIND -eq 1 ]]
then
if [ -z $VALGRINDLOGS ]
then
VALGRINDLOGS=valgrind_logs
fi
echo "valgrind --vgdb=yes --tool=memcheck --log-file=$VALGRINDLOGS --leak-check=yes --show-reachable=yes --num-callers=20 --track-fds=yes ./Spark $1 $2 $3 $4 $5 $6 $7"
if [ -z $SUPPRESSIONS ]
then
valgrind --vgdb=yes --tool=memcheck --log-file=$VALGRINDLOGS --leak-check=yes --show-reachable=yes --num-callers=20 --track-fds=yes ./Spark $1 $2 $3 $4 $5 $6 $7
else
valgrind --vgdb=yes --tool=memcheck --suppressions=$SUPPRESSIONS  --log-file=$VALGRINDLOGS --leak-check=yes --show-reachable=yes --num-callers=20 --track-fds=yes ./Spark $1 $2 $3 $4 $5 $6 $7
fi
else
./Spark $1 $2 $3 $4 $5 $6 $7
fi
#To run Spark as background process
#./Spark $1 $2 $3 $4 $5 $6 $7 < `tty` >> /var/tmp/spark.log 2>&1 &

# Development stuff...
#

#valgrind --tool=memcheck --leak-check=yes --show-reachable=yes --num-callers=20 --track-fds=yes ./Spark $1 $2 $3 $4 $5 $6 $7
#./Spark $1 $2 $3 $4 $5 $6 $7

#valgrind --tool=callgrind ./Spark $1 $2

# NOTE: (rough) Process Performance Timing
#time ./Spark  $1 $2 -R

# NOTE:  To use GDB ... use the DEBUG .so path for libnode
#gdb --args Spark  $1 $2 $3 $4

#strace -o trace.txt Spark $1 $2 $3 $4 $5 $6 $7
