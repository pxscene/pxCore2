#!/bin/sh

cd $TRAVIS_BUILD_DIR/tests/pxScene2d;
touch $TRAVIS_BUILD_DIR/logs/test_logs;
TESTLOGS=$TRAVIS_BUILD_DIR/logs/test_logs;
sudo ./pxscene2dtests.sh>$TESTLOGS 2>&1;
grep "FAILED TESTS" $TESTLOGS
retVal=$?
if [ "$retVal" -eq 0 ]
then
if [ "$TRAVIS_PULL_REQUEST" != "false" ]
then
cat $TESTLOGS
fi 
exit 1;
else
exit 0;
fi
