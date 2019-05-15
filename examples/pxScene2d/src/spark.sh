#!/bin/bash

#Get absolute path to this script
THIS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

cd $THIS_DIR

externalDir=../external

externalLibs=$externalDir/png/.libs/:$externalDir/jpg/.libs/:$externalDir/curl/lib/.libs/:$externalDir/ft/objs/.libs/:$externalDir/zlib:$externalDir/westeros/external/install/lib/:$externalDir/libjpeg-turbo/.libs/:rpc/:$externalDir/gif/.libs/
PathD=$externalLibs:$externalDir/libnode-v6.9.0/out/Debug/obj.target
PathR=$externalLibs:$externalDir/libnode-v6.9.0/out/Release/obj.target
PathR=$externalLibs/extlibs/lib:PathR

export LD_LIBRARY_PATH=$PathR

export NODE_PATH=.

export GST_REGISTRY_FORK="no"
export GST_PLUGIN_SCANNER=$externalLibs/extlibs/libexec/gstreamer-1.0/gst-plugin-scanner
export GST_PLUGIN_PATH=$externalLibs/extlibs/lib/
export GST_REGISTRY=/tmp/.spark_gst_registry.bin

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
