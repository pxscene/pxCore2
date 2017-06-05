#!/bin/sh
#This script is used to detect leaked px objects or textures
export PX_DUMP_MEMUSAGE=1
export RT_LOG_LEVEL=info
PXCHECKLOGS=$TRAVIS_BUILD_DIR/logs/pxcheck_logs

rm -rf /var/tmp/pxscene.log
cd $TRAVIS_BUILD_DIR/examples/pxScene2d/src/pxscene.app/Contents/MacOS
./pxscene.sh testRunner_memcheck.js?tests=$TRAVIS_BUILD_DIR/tests/pxScene2d/testRunner/tests.json &
grep "RUN COMPLETED" /var/tmp/pxscene.log
retVal=$?
count=0
while [ "$retVal" -ne 0 ] &&  [ "$count" -ne 5400 ]; do
sleep 30;
grep "RUN COMPLETED" /var/tmp/pxscene.log
retVal=$?
count=$((count+30))
done

kill -15 `ps -ef | grep pxscene |grep -v grep|grep -v pxscene.sh|awk '{print $2}'`
echo "Sleeping to make terminate complete ......";
sleep 5s;
pkill -9 -f pxscene.sh
cp /var/tmp/pxscene.log $PXCHECKLOGS
grep "pxobjectcount is \[0\]" $PXCHECKLOGS
pxRetVal=$?
grep "texture memory usage is \[0\]" $PXCHECKLOGS
texRetVal=$?
if [[ "$pxRetVal" == 0 ]] && [[ "$texRetVal" == 0 ]] ; then
exit 0;
else
echo "!!!!!!!!!!!!! pxobject leak or texture leak present !!!!!!!!!!!!!!!!";
if [ "$TRAVIS_PULL_REQUEST" != "false" ]
then
cat $PXCHECKLOGS
fi 
exit 1;
fi
