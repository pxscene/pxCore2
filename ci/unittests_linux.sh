#!/bin/sh

checkError()
{
  if [ "$1" -ne 0 ]
  then
    printf "\n\n*********************************************************************";
    printf "\n*******************CODE COVERAGE FAIL DETAILS************************";
    printf "\nCI failure reason: $2"
    printf "\nCause:  $3"
    printf "\nReproduction/How to fix: $4"	
    printf "\n*********************************************************************";
    printf "\n*********************************************************************\n\n";
    exit 1
  fi
}

ulimit -c unlimited
export HANDLE_SIGNALS=1
rm /tmp/pxscenecrash
cd $TRAVIS_BUILD_DIR/tests/pxScene2d;
touch $TRAVIS_BUILD_DIR/logs/test_logs;
TESTLOGS=$TRAVIS_BUILD_DIR/logs/test_logs;
./pxscene2dtests.sh>$TESTLOGS 2>&1 &

grep "Global test environment tear-down" $TESTLOGS
retVal=$?
count=0
corefile=1
while [ "$retVal" -ne 0 ] &&  [ "$count" -ne 180 ] && [ "$corefile" -eq 1 ] ; do
	sleep 60;
	grep "Global test environment tear-down" $TESTLOGS
	retVal=$?
        ls /tmp/pxscenecrash
        corefile=$? 
	count=$((count+60))
	echo "unittests running for $count seconds"
done

#check for corefile presence
processId=`ps -ef | grep pxscene2dtests |grep -v grep|grep -v pxscene2dtests.sh|awk '{print $2}'`
ls -l /tmp/pxscenecrash
retVal=$?
if [ "$retVal" -eq 0 ]
  then
  $TRAVIS_BUILD_DIR/ci/check_dump_cores_linux.sh `pwd` "$TRAVIS_BUILD_DIR/tests/pxScene2d/pxscene2dtests" "$processId" "$TESTLOGS"
  kill -9 $processId
  sleep 5s;
  pkill -9 -f pxscene2dtests.sh
  echo "********************** PRINTING TEST LOG **************************"
  cat $TESTLOGS
  echo "************************** LOG ENDS *******************************"
  checkError -1 "Unittests execution failed" "Core dump"  "Verify Unit test logs/Run unittests locally."
fi

kill -9 $processId
sleep 5s;
pkill -9 -f pxscene2dtests.sh

#check for process hung
errCause=""
grep "Global test environment tear-down" $TESTLOGS
retVal=$?
if [ "$retVal" -ne 0 ]
	then
	if [ "$TRAVIS_PULL_REQUEST" != "false" ]
		then
		errCause="Either one or more tests failed. Check the above logs"
                echo "********************** PRINTING TEST LOG **************************"
                cat $TESTLOGS
                echo "************************** LOG ENDS *******************************"
        else
		errCause="Either one or more tests failed. Check the log file $TESTLOGS"
	fi 
	checkError $retVal "unittests execution failed" "$errCause" "Run unittests locally"
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
	        echo "********************** PRINTING TEST LOG **************************"
                cat $TESTLOGS
                echo "************************** LOG ENDS *******************************"
        else
		errCause="Either one or more tests failed. Check the log file $TESTLOGS"
	fi 
	checkError -1 "unittests execution failed" "$errCause" "Run unittests locally"
else
	exit 0;
fi
