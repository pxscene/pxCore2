#!/bin/sh
sudo rm -rf /tmp/cache/*
LEAKLOGS=$TRAVIS_BUILD_DIR/logs/leak_logs
export ENABLE_MEMLEAK_CHECK=1
export PX_DUMP_MEMUSAGE=1
export MallocStackLogging=1
LEAKPXCORELOGS=$TRAVIS_BUILD_DIR/logs/leak_pxcore_logs

#run leak detector and monitor for completion
rm /var/tmp/pxscene.log
cd $TRAVIS_BUILD_DIR/examples/pxScene2d/src/pxscene.app/Contents/MacOS
./pxscene.sh testRunner_memcheck.js?tests=file://$TRAVIS_BUILD_DIR/tests/pxScene2d/testRunner/tests.json &
grep "RUN COMPLETED" /var/tmp/pxscene.log
retVal=$?
count=0
leakcount=0
while [ "$retVal" -ne 0 ] &&  [ "$count" -lt 540 ]; do
leaks -nocontext pxscene > $LEAKLOGS
grep "RUN COMPLETED" /var/tmp/pxscene.log
retVal=$?
count=$((count+20))
sleep 20;
echo "testrunner execution going on for $count seconds";
done
#kill pxscene either after js terminates or no response for 5 minutes
echo "`ps -ef | grep -v grep|grep pxscene|grep -v pxscene.sh|awk '{print $2}'`"
kill -15 `ps -ef | grep -v grep|grep pxscene|grep  -v pxscene.sh|awk '{print $2}'`
#wait for few seconds to get the application terminate completely
leakcount=`leaks pxscene|grep Leak|wc -l`
echo "leakcount during termination $leakcount"
sleep 5s;
pkill -9 -f pxscene.sh
cp /var/tmp/pxscene.log $LEAKPXCORELOGS

#check whether leak is present
if [ "$leakcount" -ne 0 ]
then
echo "CI failure reason: Valgrind execution reported memory leaks";
if [ "$TRAVIS_PULL_REQUEST" != "false" ]
then
echo "Cause: Check the below logs"
cat $LEAKLOGS
echo "Cause: Check the file $LEAKLOGS and $LEAKPXCORELOGS"
fi
echo "How to fix: run locally with these steps: export ENABLE_MEMLEAK_CHECK=1;export MallocStackLogging=1;export PX_DUMP_MEMUSAGE=1;./pxscene.sh testRunner_memcheck.js?tests=<pxcore dir>/tests/pxScene2d/testRunner/tests.json &; run leaks -nocontext pxscene >logfile continuously until the testrunner execution completes; Analyse the logfile"
exit 1;
fi
exit 0;
