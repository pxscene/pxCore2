#!/bin/sh
export VALGRINDLOGS=$TRAVIS_BUILD_DIR/logs/valgrind_logs
export ENABLE_VALGRIND=1
export PX_DUMP_MEMUSAGE=1
export SUPPRESSIONS=$TRAVIS_BUILD_DIR/leak.supp
VALGRINDPXCORELOGS=$TRAVIS_BUILD_DIR/logs/valgrind_pxcore_logs

touch $VALGRINDLOGS
#run valgrind and monitor for completion
cd $TRAVIS_BUILD_DIR/examples/pxScene2d/src
./pxscene.sh testRunner_memcheck.js > $VALGRINDPXCORELOGS &
grep "RUN COMPLETED" $VALGRINDPXCORELOGS
retVal=$?
count=0
while [ "$retVal" -ne 0 ] &&  [ "$count" -lt 600 ]; do
sleep 30;
grep "RUN COMPLETED" $VALGRINDPXCORELOGS
retVal=$?
count=$((count+30))
echo "Valgrind execution going on for $count seconds";
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
echo "Valgrind execution got stuck and not terminated in 5 minutes";
if [ "$TRAVIS_PULL_REQUEST" != "false" ]
then
cat $VALGRINDLOGS
fi
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
echo "!!!!!!!!!!!!! Memory leak present !!!!!!!!!!!!!!!!!!!";
if [ "$TRAVIS_PULL_REQUEST" != "false" ]
then
cat $VALGRINDLOGS
fi
exit 1;
fi
