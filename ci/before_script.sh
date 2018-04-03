#!/bin/sh

currdir=`pwd`
cd $TRAVIS_BUILD_DIR/tests/pxScene2d/testRunner
cp tests.json tests.json_orig

#make arrangements for ignoring pxwayland tests for osx and linux
if [ "$TRAVIS_OS_NAME" = "osx" ]
then
	sed -i -n '/pxWayland/d' tests.json
else
	sed -i '/pxWayland/d' tests.json
fi

#make arrangements for ignoring permissions tests for osx and linux
if grep -q "_TEST_PERMISSIONS_CHECK\" OFF" "$TRAVIS_BUILD_DIR/tests/pxScene2d/CMakeLists.txt"
then
	if [ "$TRAVIS_OS_NAME" = "osx" ]
	then
		sed -i -n '/permissions/d' tests.json
	else
		sed -i '/permissions/d' tests.json
	fi
fi

#delete last comma in json file, if any
sed -i -e x -e '$ {s/,$//;p;x;}' -e 1d tests.json

cd $currdir
exit 0;
