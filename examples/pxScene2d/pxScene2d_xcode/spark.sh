#!/bin/bash

#Get absolute path to this script
THIS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

cd "$THIS_DIR"

export DYLD_LIBRARY_PATH=../MacOS/lib/
export LD_LIBRARY_PATH=../MacOS/lib/

#../MacOS/pxscene $* < /dev/zero >> /var/tmp/pxscene.log 2>&1 &
../MacOS/pxScene2d $* < /dev/zero >> /var/tmp/spark.log 2>&1 &

# Software update below

# Get pid of last background process which should be pxscene
PXPID=$!

# This file is required by the .engine_install script
# so that it can wait for pxscene to shutdown prior to software update
echo $PXPID > ./lastpid

# Only try to update this bundle if a version has been included
# and if this directory is writeable (not a .dmg for example)
if [ -e ./version ] && [ -w . ]; then

echo "Info: Checking for Software Update"

VERSION=`cat ./version`
UPDATE_URL=http://www.pxscene.org/dist/osx/pxscene/software_update.plist

../MacOS/EngineRunner run -productid org.pxscene.pxscene -version $VERSION -url ${UPDATE_URL} &

else
echo "Info: No ./version file assuming dev build.  Skip software update"
fi



