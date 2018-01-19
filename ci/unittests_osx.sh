#!/bin/sh
checkError()
{
  if [ "$1" -ne 0 ]
  then
    printf "\n\n*********************************************************************";
    printf "\n*******************CODE COVERAGE FAIL DETAILS************************";
    printf "\nCI failure reason: $2"
    printf "\nCause: $3"
    printf "\nReproduction/How to fix: $4"
    printf "\n*********************************************************************";
    printf "\n*********************************************************************\n\n";
    exit 1
  fi
}

ulimit -c unlimited

cd $TRAVIS_BUILD_DIR
cored=0
export HANDLE_SIGNALS=1
export RT_LOG_LEVEL=info
cd $TRAVIS_BUILD_DIR/tests/pxScene2d;
touch $TRAVIS_BUILD_DIR/logs/test_logs;
TESTLOGS=$TRAVIS_BUILD_DIR/logs/test_logs;
#sudo -E ./pxscene2dtests.sh>$TESTLOGS 2>&1 &
./pxscene2dtests.sh>$TESTLOGS 2>&1 &

grep "Global test environment tear-down" $TESTLOGS
retVal=$?
count=0
while [ "$retVal" -ne 0 ] &&  [ "$count" -ne 300 ]; do
	sleep 60;
	grep "Global test environment tear-down" $TESTLOGS
	retVal=$?
	count=$((count+60))
	echo "unittests running for $count seconds"

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
done

#check for corefile presence
echo "core happened during unittests execution - $cored"
if [ "$cored" -eq 1 ]
then
	$TRAVIS_BUILD_DIR/ci/check_dump_cores_osx.sh `pwd` `ps -ef | grep pxscene2dtests |grep -v grep|grep -v pxscene2dtests.sh|awk '{print $2}'` $TESTLOGS
	retVal=$?
	if [ "$retVal" -eq 1 ]
	then
		checkError $retVal "unittests execution failed" "Core dump" "Run unittests locally"
	fi
fi

ps -ef | grep -i pxscene2dtests
echo "kill -9 `ps -ef | grep pxscene2dtests |grep -v grep|grep -v pxscene2dtests.sh|awk '{print $2}'`"
kill -9 `ps -ef | grep pxscene2dtests |grep -v grep|grep -v pxscene2dtests.sh|awk '{print $2}'`
sleep 5s;
pkill -9 -f pxscene2dtests.sh

errCause=""
#check for process hung
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
	checkError $retVal "unittests execution failed" "$errCause" "Rrun unittests locally"
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
