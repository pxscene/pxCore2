#!/bin/sh
export PX_DUMP_MEMUSAGE=1
export ENABLE_VALGRIND=0
export RT_LOG_LEVEL=info
PXCHECKLOGS=$TRAVIS_BUILD_DIR/logs/pxcheck_logs
cd $TRAVIS_BUILD_DIR/examples/pxScene2d/src
./pxscene.sh testRunner_memcheck.js?tests=file://$TRAVIS_BUILD_DIR/tests/pxScene2d/testRunner/tests.json > $PXCHECKLOGS 2>&1 &
grep "RUN COMPLETED" $PXCHECKLOGS
retVal=$?
count=0
while [ "$retVal" -ne 0 ] &&  [ "$count" -ne 5400 ]; do
sleep 30;
grep "RUN COMPLETED" $PXCHECKLOGS
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
sleep 5s;
pkill -9 -f pxscene.sh
$TRAVIS_BUILD_DIR/ci/check_dump_cores.sh `pwd` pxscene $PXCHECKLOGS
grep "pxobjectcount is \[0\]" $PXCHECKLOGS
pxRetVal=$?
grep "texture memory usage is \[0\]" $PXCHECKLOGS
texRetVal=$?
echo "Values are $pxRetVal and $texRetVal";
if [ "$pxRetVal" -eq 0 ]
then
echo "pxobject count success *****************";
if [ "$texRetVal" -eq 0 ]
then
echo "texture size success *****************";
exit 0;
fi
echo "!!!!!!!!!!!!! texture size leaked !!!!!!!!!!!!!!!!";
echo "CI failure reason: Texture leak"
if [ "$TRAVIS_PULL_REQUEST" != "false" ]
then
echo "Cause: Check the below logs"
cat $PXCHECKLOGS
else
echo "Cause: Check the $PXCHECKLOGS file"
fi
echo "Reproduction/How to fix: Follow steps locally: export PX_DUMP_MEMUSAGE=1;export RT_LOG_LEVEL=info;./pxscene.sh testRunner_memcheck.js?tests=<pxcore dir>/tests/pxScene2d/testRunner/tests.json locally and check for 'texture memory usage is' in logs. Analyze why the usage is not 0"
exit 1;
else
echo "!!!!!!!!!!!!! pxobject leaked !!!!!!!!!!!!!!!!";
echo "CI failure reason: pxobject leak"
if [ "$TRAVIS_PULL_REQUEST" != "false" ]
then
echo "Cause: Check the below logs"
cat $PXCHECKLOGS
else
echo "Cause: Check the $PXCHECKLOGS file"
fi
echo "Reproduction/How to fix: Follow steps locally: export PX_DUMP_MEMUSAGE=1;export RT_LOG_LEVEL=info;./pxscene.sh testRunner_memcheck.js?tests=<pxcore dir>/tests/pxScene2d/testRunner/tests.json locally and check for 'pxobjectcount is' in logs. Analyze why the count is not 0?"
exit 1;
fi
