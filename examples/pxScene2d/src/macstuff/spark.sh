#!/bin/bash

#Get absolute path to this script
THIS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

cd "$THIS_DIR"

# Needed to pickup the spark-webgl native module
export NODE_PATH="$THIS_DIR":"$THIS_DIR/node_modules"

updateEdge=true
cmdLineArgs=false
export LD_LIBRARY_PATH=./lib/
export DYLD_LIBRARY_PATH=./lib/
export GST_REGISTRY_FORK="no"
export GST_PLUGIN_SCANNER=./lib/gst-plugin-scanner
export GST_PLUGIN_PATH=./lib/
export GST_REGISTRY=/tmp/.spark_gst_registry.bin

export SPARK_ENABLE_DEBUGGING=1
for i in $*; do 
   if [[ $i == "-autoUpdateEdge="* ]] ; 
   then
     IFS='=' tokens=( $i )
     updateEdge=`echo ${tokens[1]} | tr '[:upper:]' '[:lower:]'`
     cmdLineArgs=true
     break 
   fi 
 done

if [[ $cmdLineArgs == false ]] ;
then
  if [[ -e $HOME/.sparkSettings.json ]]; 
  then
    KEY=autoUpdateEdge
    num=1
    updateEdge=`cat $HOME/.sparkSettings.json | awk -F"[,:}]" '{for(i=1;i<=NF;i++){if($i~/'$KEY'\042/){print $(i+1)}}}' | tr -d '"' | sed -n ${num}p| tr '[:upper:]' '[:lower:]'`
    if [[ $updateEdge != "false" ]] && [[ $updateEdge != "0" ]] ; then
      updateEdge=true
    fi
  fi
fi

if [ -z ${DS+x} ]
then
echo Logging to log files
./Spark --experimental-vm-modules $* < /dev/zero >> /var/tmp/Spark.log 2>&1 &
else
echo Logging to console
./Spark --experimental-vm-modules $*
fi

# Software update below

# Get pid of last background process which should be Spark
PXPID=$!

# This file is required by the .engine_install script
# so that it can wait for Spark to shutdown prior to software update
echo $PXPID > ./lastpid

# Only try to update this bundle if a version has been included
# and if this directory is writeable (not a .dmg for example)
if [ -e ./version ] && [ -w . ]; then
  echo "Info: Checking for Software Update"
  VERSION=`cat ./version`
  if [[ "$VERSION" != "edge"* ]]; then
    UPDATE_URL=http://www.sparkui.org/dist/osx/spark/software_update.plist
    ./EngineRunner run -productid org.pxscene.pxscene -version $VERSION -url ${UPDATE_URL} &
  else
    if [[ $updateEdge == "true" ]] || [[ $updateEdge == 1 ]] ; then
      UPDATE_URL=http://96.116.56.119/edge/osx/artifacts/software_update.plist
      ./EngineRunner run -productid org.pxscene.pxscene -version $VERSION -url ${UPDATE_URL} &
    fi
  fi
else
    echo "Info: No ./version file assuming dev build.  Skip software update"
fi

