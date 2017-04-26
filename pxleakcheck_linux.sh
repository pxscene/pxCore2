#!/bin/sh
export PX_DUMP_MEMUSAGE=1
export ENABLE_VALGRIND=0
export RT_LOG_LEVEL=info
PXCHECKLOGS=$TRAVIS_BUILD_DIR/logs/pxcheck_logs
cd $TRAVIS_BUILD_DIR/examples/pxScene2d/src
./pxscene.sh testRunner_memcheck.js > $PXCHECKLOGS 2>&1 &
grep "RUN COMPLETED" $PXCHECKLOGS
retVal=$?
count=0
while [ "$retVal" -ne 0 ] &&  [ "$count" -ne 5400 ]; do
sleep 30;
grep "RUN COMPLETED" $PXCHECKLOGS
retVal=$?
count=$((count+30))
done

kill -15 `ps -ef | grep pxscene |grep -v grep|grep -v pxscene.sh|awk '{print $2}'`
echo "Sleeping to make terminate complete ......";
sleep 5s;
pkill -9 -f pxscene.sh
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
if [ "$TRAVIS_PULL_REQUEST" != "false" ]
then
cat $PXCHECKLOGS
fi
exit 1;
else
echo "!!!!!!!!!!!!! pxobject leaked !!!!!!!!!!!!!!!!";
if [ "$TRAVIS_PULL_REQUEST" != "false" ]
then
cat $PXCHECKLOGS
fi
exit 1;
fi
