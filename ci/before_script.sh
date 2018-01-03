#!/bin/sh
#make arrangements for ignoring pxwayland tests for osx and linux
currdir = `pwd`
cd $TRAVIS_BUILD_DIR/tests/pxScene2d/testRunner
cp tests.json tests.json_orig
if [ "$TRAVIS_OS_NAME" = "osx" ]
then
	sed -i -n '/pxWayland/d' tests.json
else
	sed -i '/pxWayland/d' tests.json
fi
cd $currdir
exit 0;
