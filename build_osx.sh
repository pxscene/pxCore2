#!/bin/sh
BUILDLOGS=$TRAVIS_BUILD_DIR/logs/build_logs
cd $TRAVIS_BUILD_DIR/src
echo "***************************** Building pxcore ****" >> $BUILDLOGS
cd $TRAVIS_BUILD_DIR;
xcodebuild -scheme "pxCore Static Library">>$BUILDLOGS 2>&1;
cd $TRAVIS_BUILD_DIR/examples/pxScene2d/src;
echo "***************************** Building pxscene app ***" >> $BUILDLOGS
make -j>>$BUILDLOGS 2>&1
