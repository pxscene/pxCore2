#!/bin/bash

#Get absolute path to this script
THIS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

cd "$THIS_DIR"

export DYLD_LIBRARY_PATH=./lib/
export LD_LIBRARY_PATH=./lib/

./Spark $* < /dev/zero >> /var/tmp/spark.log 2>&1 &

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
  if [ "$VERSION" != "edge" ]; then
    UPDATE_URL=http://www.Spark.org/dist/osx/Spark/software_update.plist
    ./EngineRunner run -productid org.Spark.Spark -version $VERSION -url ${UPDATE_URL} &
  fi
else
    echo "Info: No ./version file assuming dev build.  Skip software update"
fi



