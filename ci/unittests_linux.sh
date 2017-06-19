#!/bin/sh

cd $TRAVIS_BUILD_DIR/tests/pxScene2d;
touch $TRAVIS_BUILD_DIR/logs/test_logs;
TESTLOGS=$TRAVIS_BUILD_DIR/logs/test_logs;
sudo ./pxscene2dtests.sh>$TESTLOGS 2>&1;
$TRAVIS_BUILD_DIR/ci/check_dump_cores.sh `pwd` pxscene2dtests $TESTLOGS
grep "FAILED TEST" $TESTLOGS
retVal=$?
if [ "$retVal" -eq 0 ]
then
echo "CI failure reason: unittests execution failed"
if [ "$TRAVIS_PULL_REQUEST" != "false" ]
then
echo "Cause: Either one or more tests failed. Check the below logs"
cat $TESTLOGS
else
echo "Cause: Either one or more tests failed. Check the log file $TESTLOGS"
fi 
echo "Reproduction/How to fix: run unittests locally"
exit 1;
else
exit 0;
fi
