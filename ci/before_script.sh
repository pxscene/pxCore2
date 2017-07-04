#!/bin/sh
#make arrangements for ignoring pxwayland tests for osx
if [ "$TRAVIS_OS_NAME" = "osx" ]
then
currdir = `pwd`
cd $TRAVIS_BUILD_DIR/tests/pxScene2d/testRunner
cp tests.json tests.json_orig
sed -i -n '/pxWayland/d' tests.json
cd $currdir
fi
exit 0;
