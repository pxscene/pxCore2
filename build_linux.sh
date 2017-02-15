#!/bin/sh
BUILDLOGS=$TRAVIS_BUILD_DIR/logs/build_logs
cd $TRAVIS_BUILD_DIR/src
echo "***************************** Building pxcore and rtcore ****" >> $BUILDLOGS
make -f Makefile.glut all>>$BUILDLOGS 2>&1;
make -f Makefile.glut rtcore>>$BUILDLOGS 2>&1;
echo "***************************** Building libpxscene ****" >> $BUILDLOGS;
cd $TRAVIS_BUILD_DIR/examples/pxScene2d/src
make libs-glut>>$BUILDLOGS 2>&1;
echo "***************************** Building pxscene app ***" >> $BUILDLOGS
make -j>>$BUILDLOGS 2>&1
echo "***************************** Building unittests ***" >> $BUILDLOGS;
cd $TRAVIS_BUILD_DIR/tests/pxScene2d;
make>>$BUILDLOGS 2>&1;
touch $TRAVIS_BUILD_DIR/logs/test_logs;
TESTLOGS=$TRAVIS_BUILD_DIR/logs/test_logs;
./pxscene2dtests.sh>$TESTLOGS 2>&1;

