#!/bin/sh

checkError()
{
  if [ "$1" -ne 0 ]
  then
        printf "\n\n*******************************************************************";
	printf "\n******************* BUILD FAIL DETAILS ******************************";
        printf "\nCI Failure reason: $2"
        printf "\nCause: $3"
        printf "\nReproduction/How to fix: $4"
	printf "\n*******************************************************************";
	printf "\n*******************************************************************\n\n";
        #exit 1;
  fi
}

#This script executes necessary javascript files and measures pxleak checks and memory leaks checks

if [ -z "${TRAVIS_BUILD_DIR}" ]
then
  printf "\nFATAL ERROR:  'TRAVIS_BUILD_DIR' env var is NOT defined\n\n"
  exit 1;
else
  printf "\nUSING: TRAVIS_BUILD_DIR=${TRAVIS_BUILD_DIR}\n\n"
fi

rm -rf /tmp/cache/*
rm -rf $TRAVIS_BUILD_DIR/logs/*
rm /tmp/pxscenecrash

export VALGRINDLOGS=$TRAVIS_BUILD_DIR/logs/valgrind_logs
export PX_DUMP_MEMUSAGE=1
export HANDLE_SIGNALS=1
export ENABLE_VALGRIND=1
export RT_LOG_LEVEL=info
export SPARK_CORS_ENABLED=true
export SPARK_PERMISSIONS_CONFIG=$TRAVIS_BUILD_DIR/examples/pxScene2d/src/sparkpermissions.conf
export SPARK_PERMISSIONS_ENABLED=true
export SUPPRESSIONS=$TRAVIS_BUILD_DIR/ci/leak.supp
export SPARK_ENABLE_COLLECT_GARBAGE=1

touch $VALGRINDLOGS
EXECLOGS=$TRAVIS_BUILD_DIR/logs/exec_logs
TESTRUNNERURL="https://px-apps.sys.comcast.net/pxscene-samples/examples/px-reference/test-run/testRunner_v7.js"
TESTS="file://$TRAVIS_BUILD_DIR/tests/pxScene2d/testRunner/tests.json,file://$TRAVIS_BUILD_DIR/tests/pxScene2d/testRunner/testsDesktop.json"

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
printExecLogs()
{
  printf "\n********************** PRINTING EXEC LOG **************************\n"
  cat $EXECLOGS
  printf "\n**********************     LOG ENDS      **************************\n"
}
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
printValgrindLogs()
{
  printf "\n********************** PRINTING VALGRIND LOG **************************\n"
  tail -150 $VALGRINDLOGS
  printf "\n**********************     LOG ENDS      **************************\n"
}
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

# Start testRunner ... 
cd $TRAVIS_BUILD_DIR/examples/pxScene2d/src
./spark.sh $TESTRUNNERURL?tests=$TESTS > $EXECLOGS 2>&1 &


grep "TEST RESULTS: " $EXECLOGS
retVal=$?

# Monitor testRunner ...
count=0
max_seconds=1500
while [ "$retVal" -ne 0 ] &&  [ "$count" -ne "$max_seconds" ]; do
	printf "\n [execute_linux.sh] snoozing for 30 seconds (%d of %d) \n" $count $max_seconds
	sleep 30; # seconds
	grep "TEST RESULTS: " $EXECLOGS
	retVal=$?
	
	count=$((count+30))
	if [ "$retVal" -ne 0 ]
		then
		ls -lrt /tmp/pxscenecrash
		retVal=$?
	fi
done

ls -lrt /tmp/pxscenecrash
retVal=$?
if [ "$retVal" -eq 0 ]
then
gdb $TRAVIS_BUILD_DIR/examples/pxScene2d/src/Spark -batch -q -ex "target remote | vgdb" -ex "thread apply all bt" -ex "quit"
fi

kill -15 `ps -ef | grep Spark |grep -v grep|grep -v spark.sh|awk '{print $2}'`
echo "Sleeping for 90 secomds to make the logs dump completely"
sleep 90
chmod 444 $VALGRINDLOGS

ls -lrt /tmp/pxscenecrash
retVal=$?
if [ "$retVal" -eq 0 ]
  then
  if [ "$TRAVIS_PULL_REQUEST" != "false" ]
  then
    echo "****************************** CORE DUMP DETAILS ******************************"
    cat $VALGRINDLOGS | head -150
    echo "*******************************************************************************"
    checkError -1 "Execution failed" "Core dump" "Kindly refer the above trace and test by running locally"
  else
    checkError -1 "Execution failed" "Core dump" "Kindly refer $VALGRINDLOGS and test by running locally"
  fi
  exit 1;
fi


# Check for any testRunner failures
grep "Failures: 0" $EXECLOGS
testRunnerRetVal=$?   # Will return 1 if NOT found
errCause=""

if [ "$testRunnerRetVal" -ne 0 ]
	then
	if [ "$TRAVIS_PULL_REQUEST" != "false" ]
		then
		errCause="Cause: Check the above logs"
		printExecLogs
	else
		errCause="Cause: Check the $EXECLOGS file"
	fi
	checkError $testRunnerRetVal "Testrunner failure" "$errCause" "Follow the steps locally: export PX_DUMP_MEMUSAGE=1;export RT_LOG_LEVEL=info;./spark.sh $TESTRUNNERURL?tests=<pxcore dir>/tests/pxScene2d/testRunner/tests.json locally and check for 'Failures: 0' in logs. Analyze whether failures is present or not"
	exit 1;
fi

# Check for pxobject or texture memory leaks
grep "pxobjectcount is \[0\]" $EXECLOGS
pxRetVal=$?
grep "texture memory usage is \[0\]" $EXECLOGS
texRetVal=$?
echo "Values are $pxRetVal and $texRetVal";

printf "\n\n -------------------------------- \n\n"


if [ "$pxRetVal" -eq 0 ]
	then
	echo "************************** pxobject count success **************************";

	if [ "$texRetVal" -eq 0 ]
		then
		echo "*************************** texture size success ***************************";
	else
		if [ "$TRAVIS_PULL_REQUEST" != "false" ]
			then
			errCause="Check the above logs"
			printExecLogs
		else
			errCause="Check the $EXECLOGS file"
		fi
		checkError $texRetVal "Texture leak" "$errCause" "Follow the steps locally: export PX_DUMP_MEMUSAGE=1;export RT_LOG_LEVEL=info;./spark.sh $TESTRUNNERURL?tests=<pxcore dir>/tests/pxScene2d/testRunner/tests.json locally and check for 'texture memory usage is' in logs. Analyze why the usage is not 0" 
		exit 1;
	fi
else
	if [ "$TRAVIS_PULL_REQUEST" != "false" ]
		then
		errCause="Check the above logs"
		printExecLogs
	else
		errCause="Check the $EXECLOGS file"
	fi
	checkError $pxRetVal "pxobject leak" "$errCause" "Follow the steps locally: export PX_DUMP_MEMUSAGE=1;export RT_LOG_LEVEL=info;./spark.sh $TESTRUNNERURL?tests=<pxcore dir>/tests/pxScene2d/testRunner/tests.json locally and check for 'pxobjectcount is' in logs. Analyze why the count is not 0?"
	exit 1;
fi

# Check for valgrind memory leaks
grep "definitely lost: 0 bytes in 0 blocks" $VALGRINDLOGS
retVal=$?
if [ "$retVal" -eq 0 ]
	then
	echo "************************* Valgrind reports success *************************";
else
	grep -A 100 -B 100 "definitely lost:" $VALGRINDLOGS
	leakcheck=$?
	if [ "$leakcheck" -eq 0 ]
	then
		errCause="Memory leaks present"
	else
		errCause="Execution stopped due to crash or abnormal execution"
	fi
	if [ "$TRAVIS_PULL_REQUEST" != "false" ]
	then
		errCause="$errCause . Check the above logs"
		printValgrindLogs
	else
		errCause="$errCause . Check the file $VALGRINDLOGS "
	fi
	checkError $retVal "Valgrind execution reported problem" "$errCause" "Follow the steps locally : export ENABLE_VALGRIND=1;export SUPPRESSIONS=<pxcore dir>/ci/leak.supp;./spark.sh $TESTRUNNERURL?tests=<pxcore dir>/tests/pxScene2d/testRunner/tests.json and fix it"
	exit 1;
fi
exit 0;
