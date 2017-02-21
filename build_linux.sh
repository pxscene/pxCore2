#!/bin/sh
BUILDLOGS=$TRAVIS_BUILD_DIR/logs/build_logs
cd $TRAVIS_BUILD_DIR/src
if [ "$TRAVIS_PULL_REQUEST" == "false" ]
then
echo "***************************** Building pxcore and rtcore ****" >> $BUILDLOGS
make clean;
make -f Makefile.glut all>>$BUILDLOGS 2>&1;
make -f Makefile.glut rtcore>>$BUILDLOGS 2>&1;
else
echo "***************************** Building pxcore and rtcore ****"
make clean;
make -f Makefile.glut all 1>>$BUILDLOGS;
make -f Makefile.glut rtcore 1>>$BUILDLOGS;
fi

cd $TRAVIS_BUILD_DIR/examples/pxScene2d/src
if [ "$TRAVIS_PULL_REQUEST" == "false" ]
then
echo "***************************** Building libpxscene ****" >> $BUILDLOGS;
make clean;
make libs-glut>>$BUILDLOGS 2>&1;
else
echo "***************************** Building libpxscene ****";
make clean;
make libs-glut 1>>$BUILDLOGS;
fi

if [ "$TRAVIS_PULL_REQUEST" == "false" ]
then
echo "***************************** Building pxscene app ***" >> $BUILDLOGS
make -j>>$BUILDLOGS 2>&1
else
echo "***************************** Building pxscene app ***"
make -j 1>>$BUILDLOGS
fi

cd $TRAVIS_BUILD_DIR/tests/pxScene2d;
if [ "$TRAVIS_PULL_REQUEST" == "false" ]
then
echo "***************************** Building unittests ***" >> $BUILDLOGS;
make clean;
make>>$BUILDLOGS 2>&1;
else
echo "***************************** Building unittests ***";
make clean;
make 1>>$BUILDLOGS;
fi

touch $TRAVIS_BUILD_DIR/logs/test_logs;
TESTLOGS=$TRAVIS_BUILD_DIR/logs/test_logs;
./pxscene2dtests.sh>$TESTLOGS 2>&1;
grep "FAILED" $TESTLOGS
retVal=$?
if [ "$retVal" -eq 0 ]
then
if [ "$TRAVIS_PULL_REQUEST" != "false" ]
then
cat $TESTLOGS
exit 1;
fi 
else
exit 0;
fi
