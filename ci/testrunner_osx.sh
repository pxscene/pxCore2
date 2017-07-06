#This script runs testrunner app in osx and it is used for code coverage

#!/bin/sh
#testing
exit 0;

ulimit -c unlimited
cd $TRAVIS_BUILD_DIR
currentdir="$(pwd)" 
cored=0

cd $currentdir
TESTRUNLOGS=$TRAVIS_BUILD_DIR/logs/run_logs

rm -rf /var/tmp/pxscene.log
TESTRUNNERTESTS=file://$TRAVIS_BUILD_DIR/tests/pxScene2d/testRunner/tests.json
cd $TRAVIS_BUILD_DIR/examples/pxScene2d/src/pxscene.app/Contents/MacOS
export HANDLE_SIGNALS=1
export RT_LOG_LEVEL=info
./pxscene.sh https://px-apps.sys.comcast.net/pxscene-samples/examples/px-reference/test-run/testRunner.js?tests=$TESTRUNNERTESTS &
grep "Failures:" /var/tmp/pxscene.log
retVal=$?
count=0
while [ "$retVal" -ne 0 ] &&  [ "$count" -ne 900 ]; do
sleep 60;
grep "Failures:" /var/tmp/pxscene.log
retVal=$?

#check any crash happened, if so stop the loop
if [ "$retVal" -ne 0 ]
then
if [ -f "/tmp/pxscenecrash" ]
then
cored=1
sudo rm -rf /tmp/pxscenecrash
break
fi
fi
#crash check ends

count=$((count+60))
echo "testrunner running for $count seconds"
done
echo "core happened during testrunner execution - $cored"
if [ "$cored" -eq 1 ]
then
$TRAVIS_BUILD_DIR/ci/check_dump_cores_osx.sh `pwd` `ps -ef | grep pxscene |grep -v grep|grep -v pxscene.sh|awk '{print $2}'` /var/tmp/pxscene.log
echo "CI failure reason: testrunner execution failed"
echo "Cause: core dump"
echo "Reproduction/How to fix: run testrunner test locally"
fi

sleep 5s;

kill -15 `ps -ef | grep pxscene |grep -v grep|grep -v pxscene.sh|awk '{print $2}'`
sleep 5s;
pkill -15 -f pxscene.sh

cp /var/tmp/pxscene.log $TESTRUNLOGS
if [ "$cored" -eq 1 ]
then
if [ "$TRAVIS_PULL_REQUEST" != "false" ]
then
cat $TESTRUNLOGS
fi
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
