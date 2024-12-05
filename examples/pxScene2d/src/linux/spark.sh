#!/bin/bash

#Get absolute path to this script
THIS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

cd "$THIS_DIR"

# Needed to pickup the spark-webgl native module
export NODE_PATH="$THIS_DIR":"$THIS_DIR/node_modules"

updateEdge=true
cmdLineArgs=false
export LD_LIBRARY_PATH=./
export DYLD_LIBRARY_PATH=./
export GST_REGISTRY_FORK="no"
export GST_PLUGIN_SCANNER=./gst-plugin-scanner
export GST_PLUGIN_PATH=./lib/
export GST_REGISTRY=/tmp/.spark_gst_registry.bin

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
