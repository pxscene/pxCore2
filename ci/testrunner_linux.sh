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
cd $TRAVIS_BUILD_DIR/examples/pxScene2d/src
./pxscene.sh https://px-apps.sys.comcast.net/pxscene-samples/examples/px-reference/test-run/testRunner.js?tests=$TESTRUNNERTESTS >> $TESTRUNLOGS 2>&1 &
grep "Failures:" $TESTRUNLOGS
retVal=$?
count=0
while [ "$retVal" -ne 0 ] &&  [ "$count" -ne 600 ]; do
sleep 60;
grep "Failures:" $TESTRUNLOGS
retVal=$?
count=$((count+60))
echo "testrunner running for $count seconds"
done

echo "kill -15 `ps -ef | grep pxscene |grep -v grep|grep -v pxscene.sh|awk '{print $2}'`"
kill -15 `ps -ef | grep pxscene |grep -v grep|grep -v pxscene.sh|awk '{print $2}'`
sleep 5s;
pkill -15 -f pxscene.sh



grep "Failures: 0" $TESTRUNLOGS
retVal=$?
if [ "$retVal" -eq 0 ]
then
exit 0;
else
if [ "$TRAVIS_PULL_REQUEST" != "false" ]
then
cat $TESTRUNLOGS
fi 
exit 1;
fi
