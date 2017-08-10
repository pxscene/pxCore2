#!/bin/sh
#This script is used to detect leaked px objects or textures
exit 0;
ulimit -c unlimited
cored=0
export PX_DUMP_MEMUSAGE=1
export RT_LOG_LEVEL=info
export HANDLE_SIGNALS=1
PXCHECKLOGS=$TRAVIS_BUILD_DIR/logs/pxcheck_logs

rm -rf /var/tmp/pxscene.log
cd $TRAVIS_BUILD_DIR/examples/pxScene2d/src/pxscene.app/Contents/MacOS
./pxscene.sh $TRAVIS_BUILD_DIR/testRunner_memcheck_$TRAVIS_OS_NAME.js?tests=file://$TRAVIS_BUILD_DIR/tests/pxScene2d/testRunner/tests.json &
grep "RUN COMPLETED" /var/tmp/pxscene.log
retVal=$?
count=0
while [ "$retVal" -ne 0 ] &&  [ "$count" -ne 5400 ]; do
sleep 30;
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

count=$((count+30))
done

echo "core happened during pxleak checks - $cored"
if [ "$cored" -eq 1 ]
then
$TRAVIS_BUILD_DIR/ci/check_dump_cores_osx.sh `pwd` `ps -ef | grep pxscene |grep -v grep|grep -v pxscene.sh|awk '{print $2}'` /var/tmp/pxscene.log
echo "CI failure reason: pxleakcheck execution failed"
echo "Cause: core dump"
echo "Reproduction/How to fix: run pxleakcheck test locally"
fi

kill -15 `ps -ef | grep pxscene |grep -v grep|grep -v pxscene.sh|awk '{print $2}'`
echo "Sleeping to make terminate complete ......";
sleep 5s;
pkill -9 -f pxscene.sh
cp /var/tmp/pxscene.log $PXCHECKLOGS
if [ "$cored" -eq 1 ]
then
exit 1;
fi
grep "pxobjectcount is \[0\]" $PXCHECKLOGS
pxRetVal=$?
grep "texture memory usage is \[0\]" $PXCHECKLOGS
texRetVal=$?
if [[ "$pxRetVal" == 0 ]] && [[ "$texRetVal" == 0 ]] ; then
exit 0;
else
echo "!!!!!!!!!!!!! pxobject leak or texture leak present !!!!!!!!!!!!!!!!";
echo "CI failure reason: Texture leak or pxobject leak"
if [ "$TRAVIS_PULL_REQUEST" != "false" ]
then
echo "Cause: Check the below logs"
cat $PXCHECKLOGS
else
echo "Cause: Check the $PXCHECKLOGS file"
fi 
echo "Reproduction/How to fix: Follow steps locally: export PX_DUMP_MEMUSAGE=1;export RT_LOG_LEVEL=info;./pxscene.sh testRunner_memcheck.js?tests=<pxcore dir>/tests/pxScene2d/testRunner/tests.json locally and check for 'texture memory usage is' and 'pxobjectcount is' in logs and see which is non-zero"
exit 1;
fi
