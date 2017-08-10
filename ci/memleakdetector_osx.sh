#!/bin/sh
ulimit -c unlimited
cored=0
sudo rm -rf /tmp/cache/*
LEAKLOGS=$TRAVIS_BUILD_DIR/logs/leak_logs
export RT_LOG_LEVEL=info
export ENABLE_MEMLEAK_CHECK=1
export PX_DUMP_MEMUSAGE=1
export MallocStackLogging=1
export HANDLE_SIGNALS=1
LEAKPXCORELOGS=$TRAVIS_BUILD_DIR/logs/leak_pxcore_logs

#run leak detector and monitor for completion
rm /var/tmp/pxscene.log
cd $TRAVIS_BUILD_DIR/examples/pxScene2d/src/pxscene.app/Contents/MacOS
./pxscene.sh $TRAVIS_BUILD_DIR/testRunner_memcheck_$TRAVIS_OS_NAME.js?tests=file://$TRAVIS_BUILD_DIR/tests/pxScene2d/testRunner/tests.json &
grep "RUN COMPLETED" /var/tmp/pxscene.log
retVal=$?
count=0
leakcount=0
while [ "$retVal" -ne 0 ] &&  [ "$count" -lt 900 ]; do
leaks -nocontext pxscene > $LEAKLOGS
grep "RUN COMPLETED" /var/tmp/pxscene.log
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

count=$((count+20))
sleep 20;
echo "testrunner execution going on for $count seconds";
done

echo "core happened during memory leak detection - $cored"
if [ "$cored" -eq 1 ]
then
$TRAVIS_BUILD_DIR/ci/check_dump_cores_osx.sh `pwd` `ps -ef | grep pxscene |grep -v grep|grep -v pxscene.sh|awk '{print $2}'` /var/tmp/pxscene.log
echo "CI failure reason: memleakdetector execution failed"
echo "Cause: core dump"
echo "Reproduction/How to fix: run memleakdetector test locally"
fi

#kill pxscene either after js terminates or no response for 5 minutes
echo "`ps -ef | grep -v grep|grep pxscene|grep -v pxscene.sh|awk '{print $2}'`"
kill -15 `ps -ef | grep -v grep|grep pxscene|grep  -v pxscene.sh|awk '{print $2}'`
if [ "$cored" -eq 1 ]
then
exit 1;
fi
#wait for few seconds to get the application terminate completely
leakcount=`leaks pxscene|grep Leak|wc -l`
echo "leakcount during termination $leakcount"
sleep 5s;
pkill -9 -f pxscene.sh
cp /var/tmp/pxscene.log $LEAKPXCORELOGS

if [ "$TRAVIS_PULL_REQUEST" != "false" ]
then
cat $LEAKPXCORELOGS
fi

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
