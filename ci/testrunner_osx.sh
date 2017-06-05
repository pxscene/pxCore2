#This script runs testrunner app in osx and it is used for code coverage
#!/bin/sh
cd $TRAVIS_BUILD_DIR
currentdir="$(pwd)" 


cd $currentdir
TESTRUNLOGS=$TRAVIS_BUILD_DIR/logs/run_logs

rm -rf /var/tmp/pxscene.log
cd $TRAVIS_BUILD_DIR/examples/pxScene2d/src/pxscene.app/Contents/MacOS
./pxscene.sh https://px-apps.sys.comcast.net/pxscene-samples/examples/px-reference/test-run/testRunner.js?tests=$TRAVIS_BUILD_DIR/tests/pxScene2d/testRunner/tests.json &
grep "Failures:" /var/tmp/pxscene.log
retVal=$?
count=0
while [ "$retVal" -ne 0 ] &&  [ "$count" -ne 480 ]; do
sleep 60;
grep "Failures:" /var/tmp/pxscene.log
retVal=$?
count=$((count+60))
done

sleep 5s;

kill -15 `ps -ef | grep pxscene |grep -v grep|grep -v pxscene.sh|awk '{print $2}'`
sleep 5s;
pkill -15 -f pxscene.sh

cp /var/tmp/pxscene.log $TESTRUNLOGS
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
