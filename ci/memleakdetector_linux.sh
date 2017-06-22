#!/bin/sh
sudo rm -rf /tmp/cache/*
export VALGRINDLOGS=$TRAVIS_BUILD_DIR/logs/valgrind_logs
export ENABLE_VALGRIND=1
export PX_DUMP_MEMUSAGE=1
export SUPPRESSIONS=$TRAVIS_BUILD_DIR/ci/leak.supp
VALGRINDPXCORELOGS=$TRAVIS_BUILD_DIR/logs/valgrind_pxcore_logs

touch $VALGRINDLOGS
#run valgrind and monitor for completion
cd $TRAVIS_BUILD_DIR/examples/pxScene2d/src
./pxscene.sh testRunner_memcheck.js?tests=file://$TRAVIS_BUILD_DIR/tests/pxScene2d/testRunner/tests.json > $VALGRINDPXCORELOGS &
grep "RUN COMPLETED" $VALGRINDPXCORELOGS
retVal=$?
count=0
while [ "$retVal" -ne 0 ] &&  [ "$count" -lt 600 ]; do
sleep 30;
grep "RUN COMPLETED" $VALGRINDPXCORELOGS
retVal=$?
count=$((count+30))
echo "Valgrind execution going on for $count seconds";
if [ "$retVal" -ne 0 ]
then
ls -lrt core
retVal=$?
fi
done

#kill pxscene either after js terminates or no response for 5 minutes
echo "`ps -ef | grep valgrind |grep -v grep|grep pxscene|grep -v pxscene.sh|awk '{print $2}'`"
kill -15 `ps -ef | grep valgrind |grep -v grep|grep pxscene|grep  -v pxscene.sh|awk '{print $2}'`
#wait for few seconds to get the application terminate completely
sleep 20s;
pkill -9 -f pxscene.sh

#check whether valgrind got completed
grep "definitely lost" $VALGRINDLOGS
retVal=$?
if [ "$retVal" -ne 0 ]
then
echo "CI failure reason: Valgrind execution got stuck and not terminated in 5 minutes";
if [ "$TRAVIS_PULL_REQUEST" != "false" ]
then
echo "Cause: Check the below logs"
cat $VALGRINDPXCORELOGS
else
echo "Cause: Check the file $VALGRINDPXCORELOGS and $VALGRINDLOGS and see where the execution is stucked"
fi
echo "How to fix: run locally with these steps: export ENABLE_VALGRIND=1;export SUPPRESSIONS=<pxcore dir>/ci/leak.supp;./pxscene.sh testRunner_memcheck.js?tests=<pxcore dir>/tests/pxScene2d/testRunner/tests.json and check wteher it is stucked or not. If stucked, fix it. Else,retry in travis again"
exit 1;
fi

$TRAVIS_BUILD_DIR/ci/check_dump_cores_linux.sh `pwd` pxscene $VALGRINDPXCORELOGS
retVal=$?
if [ "$retVal" -eq 1 ]
then
echo "CI failure reason: memleakdetection execution failed"
echo "Cause: core dump"
echo "Reproduction/How to fix: run memleakdetection test locally"
chmod 444 $VALGRINDLOGS
exit 1;
fi

chmod 444 $VALGRINDLOGS
#check for memory leak
grep "definitely lost: 0 bytes in 0 blocks" $VALGRINDLOGS
retVal=$?
if [ "$retVal" -eq 0 ]
then
exit 0;
else
echo "CI failure reason: Valgrind execution reported memory leaks";
if [ "$TRAVIS_PULL_REQUEST" != "false" ]
then
echo "Cause: Check the below logs"
cat $VALGRINDLOGS
else
echo "Cause: Check the file $VALGRINDLOGS and see for definitely lost count"
fi
echo "How to fix: run locally with these steps: export ENABLE_VALGRIND=1;export SUPPRESSIONS=<pxcore dir>/ci/leak.supp;./pxscene.sh testRunner_memcheck.js?tests=<pxcore dir>/tests/pxScene2d/testRunner/tests.json and fix the leaks"
exit 1;
fi
