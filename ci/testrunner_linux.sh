# This script runs testrunner in  linux and it is used for code coverage
#!/bin/sh
cd $TRAVIS_BUILD_DIR
currentdir="$(pwd)"
cd src
lcov -d obj/ --zerocounters 
cd ../examples/pxScene2d/src
lcov -d obj/ --zerocounters 

cd $currentdir
TESTRUNLOGS=$TRAVIS_BUILD_DIR/logs/run_logs
TESTRUNNERTESTS=file://$TRAVIS_BUILD_DIR/tests/pxScene2d/testRunner/tests.json
isCored=1
cd $TRAVIS_BUILD_DIR/examples/pxScene2d/src
./pxscene.sh https://px-apps.sys.comcast.net/pxscene-samples/examples/px-reference/test-run/testRunner.js?tests=$TESTRUNNERTESTS >> $TESTRUNLOGS 2>&1 &
grep "Failures:" $TESTRUNLOGS
retVal=$?
count=0
while [ "$retVal" -ne 0 ] &&  [ "$count" -ne 900 ]; do
sleep 60;
grep "Failures:" $TESTRUNLOGS
retVal=$?
count=$((count+60))
echo "testrunner running for $count seconds"
if [ "$retVal" -ne 0 ]
then
ls -lrt core
retVal=$?
isCored=$retVal
fi
done

echo "kill -15 `ps -ef | grep pxscene |grep -v grep|grep -v pxscene.sh|awk '{print $2}'`"
kill -15 `ps -ef | grep pxscene |grep -v grep|grep -v pxscene.sh|awk '{print $2}'`
sleep 5s;
pkill -15 -f pxscene.sh

if [ "$isCored" -ne 0 ]
then
rm -rf core
fi

$TRAVIS_BUILD_DIR/ci/check_dump_cores_linux.sh `pwd` pxscene $TESTRUNLOGS
retVal=$?
if [ "$retVal" -eq 1 ]
then
echo "CI failure reason: testrunner execution failed"
echo "Cause: core dump"
echo "Reproduction/How to fix: run testrunner locally"
exit 1;
fi

grep "Failures: 0" $TESTRUNLOGS
retVal=$?
if [ "$retVal" -eq 0 ]
then
exit 0;
else
echo "CI failure reason: testrunner execution failed"
if [ "$TRAVIS_PULL_REQUEST" != "false" ]
then
echo "Cause: Either one or more tests failed. Check the below logs"
cat $TESTRUNLOGS
else
echo "Cause: Either one or more tests failed. Check the log file $TESTRUNLOGS"
fi 
echo "Reproduction/How to fix: run pxscene with testrunner.js locally as ./pxscene.sh https://px-apps.sys.comcast.net/pxscene-samples/examples/px-reference/test-run/testRunner.js?tests=<pxcore dir>tests/pxScene2d/testRunner/tests.json"
exit 1;
fi
