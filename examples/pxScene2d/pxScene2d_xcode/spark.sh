#!/bin/bash

#Get absolute path to this script
SOURCE="${BASH_SOURCE[0]}"
while [ -h "$SOURCE" ]; do # resolve $SOURCE until the file is no longer a symlink
  THIS_DIR="$( cd -P "$( dirname "$SOURCE" )" >/dev/null 2>&1 && pwd )"
  SOURCE="$(readlink "$SOURCE")"
  [[ $SOURCE != /* ]] && SOURCE="$THIS_DIR/$SOURCE" # if $SOURCE was a relative symlink, we need to resolve it relative to the path where the symlink file was located
done
THIS_DIR="$( cd -P "$( dirname "$SOURCE" )" >/dev/null 2>&1 && pwd )"

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
UPDATE_URL=http://www.sparkui.org/dist/osx/spark/software_update.plist

../MacOS/EngineRunner run -productid org.pxscene.pxscene -version $VERSION -url ${UPDATE_URL} &

else
echo "Info: No ./version file assuming dev build.  Skip software update"
fi



