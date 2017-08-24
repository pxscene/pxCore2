#!/bin/sh
sudo rm -rf /tmp/cache/*
export VALGRINDLOGS=$TRAVIS_BUILD_DIR/logs/valgrind_logs
export PX_DUMP_MEMUSAGE=1
export ENABLE_VALGRIND=1
export RT_LOG_LEVEL=info
export SUPPRESSIONS=$TRAVIS_BUILD_DIR/ci/leak.supp

touch $VALGRINDLOGS
EXECLOGS=$TRAVIS_BUILD_DIR/logs/exec_logs
cd $TRAVIS_BUILD_DIR/examples/pxScene2d/src
./pxscene.sh $TRAVIS_BUILD_DIR/ci/testRunner_memcheck_$TRAVIS_OS_NAME.js?tests=file://$TRAVIS_BUILD_DIR/tests/pxScene2d/testRunner/tests.json > $EXECLOGS 2>&1 &
grep "RUN COMPLETED" $EXECLOGS
retVal=$?
count=0
while [ "$retVal" -ne 0 ] &&  [ "$count" -ne 5400 ]; do
sleep 30;
grep "RUN COMPLETED" $EXECLOGS
retVal=$?
count=$((count+30))
if [ "$retVal" -ne 0 ]
then
ls -lrt core
retVal=$?
fi
done

kill -15 `ps -ef | grep pxscene |grep -v grep|grep -v pxscene.sh|awk '{print $2}'`
echo "Sleeping to make terminate complete ......";
#wait for few seconds to get the application terminate completely, as it is attached with valgrind increasing the timeout
sleep 20s;
pkill -9 -f pxscene.sh

chmod 444 $VALGRINDLOGS

#check for crash
$TRAVIS_BUILD_DIR/ci/check_dump_cores_linux.sh `pwd` pxscene $EXECLOGS
retVal=$?
if [ "$retVal" -eq 1 ]
then
echo "CI failure reason: execution failed"
echo "Cause: core dump"
echo "Reproduction/How to fix: run execution test locally"
if [ "$TRAVIS_PULL_REQUEST" != "false" ]
then
cat $EXECLOGS
fi
exit 1;
fi

#check for testrunner failures
grep "Failures: 0" $EXECLOGS
testRunnerRetVal=$?
if [ "$testRunnerRetVal" -ne 0 ]
then
echo "CI failure reason: Testrunner failure"
if [ "$TRAVIS_PULL_REQUEST" != "false" ]
then
echo "Cause: Check the below logs"
cat $EXECLOGS
else
echo "Cause: Check the $EXECLOGS file"
fi
echo "Reproduction/How to fix: Follow steps locally: export PX_DUMP_MEMUSAGE=1;export RT_LOG_LEVEL=info;./pxscene.sh testRunner_memcheck.js?tests=<pxcore dir>/tests/pxScene2d/testRunner/tests.json locally and check for 'Failures: 0' in logs. Analyze whether failures is present or not"
exit 1;
fi

#check for pxobject leak ot texture leak
grep "pxobjectcount is \[0\]" $EXECLOGS
pxRetVal=$?
grep "texture memory usage is \[0\]" $EXECLOGS
texRetVal=$?
echo "Values are $pxRetVal and $texRetVal";
if [ "$pxRetVal" -eq 0 ]
then
echo "pxobject count success *****************";
if [ "$texRetVal" -eq 0 ]
then
echo "texture size success *****************";
fi
echo "!!!!!!!!!!!!! texture size leaked !!!!!!!!!!!!!!!!";
echo "CI failure reason: Texture leak"
if [ "$TRAVIS_PULL_REQUEST" != "false" ]
then
echo "Cause: Check the below logs"
cat $EXECLOGS
else
echo "Cause: Check the $EXECLOGS file"
fi
echo "Reproduction/How to fix: Follow steps locally: export PX_DUMP_MEMUSAGE=1;export RT_LOG_LEVEL=info;./pxscene.sh testRunner_memcheck.js?tests=<pxcore dir>/tests/pxScene2d/testRunner/tests.json locally and check for 'texture memory usage is' in logs. Analyze why the usage is not 0"
exit 1;
else
echo "!!!!!!!!!!!!! pxobject leaked !!!!!!!!!!!!!!!!";
echo "CI failure reason: pxobject leak"
if [ "$TRAVIS_PULL_REQUEST" != "false" ]
then
echo "Cause: Check the below logs"
cat $EXECLOGS
else
echo "Cause: Check the $EXECLOGS file"
fi
echo "Reproduction/How to fix: Follow steps locally: export PX_DUMP_MEMUSAGE=1;export RT_LOG_LEVEL=info;./pxscene.sh testRunner_memcheck.js?tests=<pxcore dir>/tests/pxScene2d/testRunner/tests.json locally and check for 'pxobjectcount is' in logs. Analyze why the count is not 0?"
exit 1;
fi


#check for valgrind memory leaks
grep "definitely lost: 0 bytes in 0 blocks" $VALGRINDLOGS
retVal=$?
if [ "$retVal" -eq 0 ]
then
echo "Valgrind reports success !!!!!!!!!!!"
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
exit 0;
