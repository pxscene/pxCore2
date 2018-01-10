#!/bin/sh

checkError()
{
  if [ "$1" -ne 0 ]
  then
    echo "*********************************************************************";
    echo "*******************CODE COVERAGE FAIL DETAILS************************";
    echo "CI failure reason: "$2""
    echo "Cause:  "$3""
    echo "Reproduction/How to fix: "$4""	
    echo "*********************************************************************";
    echo "*********************************************************************";
    exit 1
  fi
}

ulimit -c unlimited
cd $TRAVIS_BUILD_DIR/tests/pxScene2d;
touch $TRAVIS_BUILD_DIR/logs/test_logs;
TESTLOGS=$TRAVIS_BUILD_DIR/logs/test_logs;
./pxscene2dtests.sh>$TESTLOGS 2>&1 &

grep "Global test environment tear-down" $TESTLOGS
retVal=$?
count=0
while [ "$retVal" -ne 0 ] &&  [ "$count" -ne 180 ]; do
	sleep 60;
	grep "Global test environment tear-down" $TESTLOGS
	retVal=$?
	count=$((count+60))
	echo "unittests running for $count seconds"
done

echo "kill -9 `ps -ef | grep pxscene2dtests |grep -v grep|grep -v pxscene2dtests.sh|awk '{print $2}'`"
kill -9 `ps -ef | grep pxscene2dtests |grep -v grep|grep -v pxscene2dtests.sh|awk '{print $2}'`
sleep 5s;
pkill -9 -f pxscene2dtests.sh

#check for process hung
grep "Global test environment tear-down" $TESTLOGS
errCause=""
retVal=$?
if [ "$retVal" -ne 0 ]
	then
	if [ "$TRAVIS_PULL_REQUEST" != "false" ]
		then
		errCause="Either one or more tests failed. Check the above logs"
		echo "********************PRINTING TEST LOGS************************"
		cat $TESTLOGS
		echo "************************LOG ENDS******************************"
	else
		errCause="Either one or more tests failed. Check the log file $TESTLOGS"
	fi 
	checkError $retVal "unittests execution failed" "$errCause" "Run unittests locally"
fi

#check for corefile presence
$TRAVIS_BUILD_DIR/ci/check_dump_cores_linux.sh `pwd` pxscene2dtests $TESTLOGS
retVal=$?
if [ "$retVal" -eq 1 ]
	then
	checkError $retVal "unittests execution failed" "Core dump" "Run unittests locally"
fi

#check for failed test
grep "FAILED TEST" $TESTLOGS
retVal=$?
cd $TRAVIS_BUILD_DIR;
if [ "$retVal" -eq 0 ]
	then
	if [ "$TRAVIS_PULL_REQUEST" != "false" ]
		then
		errCause="Either one or more tests failed. Check the above logs"
		echo "********************PRINTING TEST LOGS************************"
		cat $TESTLOGS
		echo "************************LOG ENDS******************************"
	else
		errCause="Either one or more tests failed. Check the log file $TESTLOGS"
	fi 
	checkError -1 "unittests execution failed" "$errCause" "Run unittests locally"
else
	exit 0;
fi
