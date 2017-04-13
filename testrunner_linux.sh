# This script runs testrunner in  linux
#!/bin/sh
cd $TRAVIS_BUILD_DIR
currentdir="$(pwd)"

cd src
lcov -d obj/ --zerocounters 
cd ../examples/pxScene2d/src
lcov -d obj/ --zerocounters 

cd $currentdir
TESTRUNLOGS=$TRAVIS_BUILD_DIR/logs/run_logs
cd $TRAVIS_BUILD_DIR/examples/pxScene2d/src
./pxscene.sh https://px-apps.sys.comcast.net/pxscene-samples/examples/px-reference/test-run/testRunner.js >> $TESTRUNLOGS 2>&1 &
grep "Failures:" $TESTRUNLOGS
retVal=$?
count=0
while [ "$retVal" -ne 0 ] &&  [ "$count" -ne 480 ]; do
sleep 60;
grep "Failures:" $TESTRUNLOGS
retVal=$?
count=$((count+60))
done

pkill -15 -f pxscene
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
