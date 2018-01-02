#!/bin/sh
#This script executes necessary javascript files and mesaures pxleak checks and memory leaks checks
ulimit -c unlimited
cored=0
export PX_DUMP_MEMUSAGE=1
export RT_LOG_LEVEL=info
export HANDLE_SIGNALS=1
export ENABLE_MEMLEAK_CHECK=1
export MallocStackLogging=1

EXECLOGS=$TRAVIS_BUILD_DIR/logs/exec_logs
LEAKLOGS=$TRAVIS_BUILD_DIR/logs/leak_logs
TESTRUNNERURL="https://px-apps.sys.comcast.net/pxscene-samples/examples/px-reference/test-run/testRunner.js"

rm -rf /var/tmp/pxscene.log
cd $TRAVIS_BUILD_DIR/examples/pxScene2d/src/pxscene.app/Contents/MacOS
./pxscene.sh $TESTRUNNERURL?tests=file://$TRAVIS_BUILD_DIR/tests/pxScene2d/testRunner/tests.json &
grep "TEST RESULTS: " /var/tmp/pxscene.log
retVal=$?
count=0
leakcount=0
while [ "$retVal" -ne 0 ] &&  [ "$count" -ne 900 ]; do
#leaks -nocontext pxscene > $LEAKLOGS
echo "execute_osx snoozing for 30"
sleep 30;
grep "TEST RESULTS: " /var/tmp/pxscene.log
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

count=$((count+30))
done


#handle crash
echo "core happened during execution - $cored"
if [ "$cored" -eq 1 ]
then
$TRAVIS_BUILD_DIR/ci/check_dump_cores_osx.sh `pwd` `ps -ef | grep pxscene |grep -v grep|grep -v pxscene.sh|awk '{print $2}'` /var/tmp/pxscene.log
echo "CI failure reason: execution failed"
echo "Cause: core dump"
echo "Reproduction/How to fix: run execution locally"
fi

#wait for few seconds to get the application terminate completely
leakcount=`leaks pxscene|grep Leak|wc -l`
echo "leakcount during termination $leakcount"
kill -15 `ps -ef | grep pxscene |grep -v grep|grep -v pxscene.sh|awk '{print $2}'`
#sleep for 40s as we have sleep for 30s inside code to capture memory of process
echo "Sleeping to make terminate complete ......";
sleep 40s;
pkill -9 -f pxscene.sh
cp /var/tmp/pxscene.log $EXECLOGS
if [ "$cored" -eq 1 ]
then
exit 1;
fi


#check for any testrunner errors
grep "Failures: 0" $EXECLOGS
retVal=$?
if [ "$retVal" -ne 0 ]
then
echo "CI failure reason: testrunner execution failed"
if [ "$TRAVIS_PULL_REQUEST" != "false" ]
then
echo "Cause: Either one or more tests failed. Check the below logs"
#cat $EXECLOGS
else
echo "Cause: Either one or more tests failed. Check the log file $EXECLOGS"
fi
echo "Reproduction/How to fix: run pxscene with testrunner.js locally as ./pxscene.sh https://px-apps.sys.comcast.net/pxscene-samples/examples/px-reference/test-run/testRunner.js?tests=<pxcore dir>tests/pxScene2d/testRunner/tests.json"
exit 1;
fi

#check for pxobject or texture memory leaks
grep "pxobjectcount is \[0\]" $EXECLOGS
pxRetVal=$?
grep "texture memory usage is \[0\]" $EXECLOGS
texRetVal=$?
if [[ "$pxRetVal" == 0 ]] && [[ "$texRetVal" == 0 ]] ; then
echo "No pxobject leaks or texture leaks found !!!!!!!!!!!!!!"
else
echo "!!!!!!!!!!!!! pxobject leak or texture leak present !!!!!!!!!!!!!!!!";
echo "CI failure reason: Texture leak or pxobject leak"
if [ "$TRAVIS_PULL_REQUEST" != "false" ]
then
echo "Cause: Check the below logs"
cat $EXECLOGS
else
echo "Cause: Check the $EXECLOGS file"
fi 
echo "Reproduction/How to fix: Follow steps locally: export PX_DUMP_MEMUSAGE=1;export RT_LOG_LEVEL=info;./pxscene.sh https://px-apps.sys.comcast.net/pxscene-samples/examples/px-reference/test-run/testRunner.js?tests=<pxcore dir>/tests/pxScene2d/testRunner/tests.json locally and check for 'texture memory usage is' and 'pxobjectcount is' in logs and see which is non-zero"
exit 1;
fi

#check for memory leaks
if [ "$leakcount" -ne 0 ]
then
echo "CI failure reason: execution reported memory leaks";
if [ "$TRAVIS_PULL_REQUEST" != "false" ]
then
echo "Cause: Check the below logs"
#cat $LEAKLOGS
else
echo "Cause: Check the file $LEAKLOGS and $EXECLOGS"
fi
echo "How to fix: run locally with these steps: export ENABLE_MEMLEAK_CHECK=1;export MallocStackLogging=1;export PX_DUMP_MEMUSAGE=1;./pxscene.sh https://px-apps.sys.comcast.net/pxscene-samples/examples/px-reference/test-run/testRunner.js?tests=<pxcore dir>/tests/pxScene2d/testRunner/tests.json &; run leaks -nocontext pxscene >logfile continuously until the testrunner execution completes; Analyse the logfile"
exit 1;
fi
exit 0;
