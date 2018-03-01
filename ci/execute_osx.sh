#!/bin/sh
#This script executes necessary javascript files and mesaures pxleak checks and memory leaks checks

if [ -z "${TRAVIS_BUILD_DIR}" ]
then
  printf "\nFATAL ERROR:  'TRAVIS_BUILD_DIR' env var is NOT defined\n\n"
  exit 1;
else
  printf "\nUSING: TRAVIS_BUILD_DIR=${TRAVIS_BUILD_DIR}\n\n"
fi

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
checkError()
{
  if [ "$1" -ne 0 ]
  then
        printf "\n\n*********************************************************************";
        printf "\n********************* BUILD FAIL DETAILS ******************************";
        printf "\nCI failure reason: $2"
        printf "\nCause: $3"
        printf "\nReproduction/How to fix: $4"
        printf "\n*********************************************************************";
        printf "\n*********************************************************************\n\n";
        exit 1;
  fi
}
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

sudo rm -rf /tmp/pxscenecrash
ulimit -c unlimited
dumped_core=0

export PX_DUMP_MEMUSAGE=1
export RT_LOG_LEVEL=info
export PXSCENE_PERMISSIONS_CONFIG=$TRAVIS_BUILD_DIR/examples/pxScene2d/src/pxscenepermissions.conf
export HANDLE_SIGNALS=1
export ENABLE_MEMLEAK_CHECK=1
export MallocStackLogging=1

EXECLOGS=$TRAVIS_BUILD_DIR/logs/exec_logs
LEAKLOGS=$TRAVIS_BUILD_DIR/logs/leak_logs
TESTRUNNERURL="https://px-apps.sys.comcast.net/pxscene-samples/examples/px-reference/test-run/testRunner.js"

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
printExecLogs()
{
  printf "\n********************** PRINTING EXEC LOG **************************\n"
  cat $EXECLOGS
  printf "\n**********************     LOG ENDS      **************************\n"
}
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

# Start testRunner ...
rm -rf /var/tmp/pxscene.log
cd $TRAVIS_BUILD_DIR/examples/pxScene2d/src/pxscene.app/Contents/MacOS
./pxscene.sh $TESTRUNNERURL?tests=file://$TRAVIS_BUILD_DIR/tests/pxScene2d/testRunner/tests.json &

# Monitor testRunner ...
count=0
max_seconds=1200

while [ "$count" -le "$max_seconds" ]; do
	#leaks -nocontext pxscene > $LEAKLOGS
	printf "\n [execute_osx.sh] snoozing for 30 seconds (%d of %d) \n" $count $max_seconds
	sleep 30; # seconds

	grep "TEST RESULTS: " /var/tmp/pxscene.log   # string in [results.js] must be "TEST RESULTS: "
	retVal=$?

	if [ "$retVal" -eq 0 ] # text found    exit code from Grep is '1' if NOT found
	then
		printf "\n ############  TESTING COMPLETE ... finishing up.\n\n"
		break
	else
	    #check any crash happened, if so stop the loop
		if [ -f "/tmp/pxscenecrash" ] # 'indicator' file created by Signal Handles in pxscene.app
		then
			printf "\n ############  CORE DUMP detected !!\n\n"
			dumped_core=1
			sudo rm -rf /tmp/pxscenecrash
			break
		fi
		#crash check ends
	fi

	count=$((count+30)) # add 30 seconds
done #LOOP

# Handle crash - 'dumped_core = 1' ?
if [ "$dumped_core" -eq 1 ]
	then
	$TRAVIS_BUILD_DIR/ci/check_dump_cores_osx.sh `pwd` `ps -ef | grep pxscene |grep -v grep|grep -v pxscene.sh|awk '{print $2}'` /var/tmp/pxscene.log
	checkError $cored "Execution failed" "Core dump" "Run execution locally"
fi

# Wait for few seconds to get the application terminate completely
leakcount=`leaks pxscene|grep Leak|wc -l`
echo "leakcount during termination $leakcount"
kill -15 `ps -ef | grep pxscene |grep -v grep|grep -v pxscene.sh|awk '{print $2}'`

# Sleep for 40s as we have sleep for 30s inside code to capture memory of process
echo "Sleeping to make terminate complete ...";
sleep 40s
pkill -9 -f pxscene.sh
cp /var/tmp/pxscene.log $EXECLOGS
if [ "$dumped_core" -eq 1 ]
	then
	echo "ERROR:  Core Dump - exiting ...";
	exit 1;
fi

# Check for any testRunner failures
errCause=""
grep "Failures: 0" $EXECLOGS
retVal=$?
if [ "$retVal" -ne 0 ]
	then
	if [ "$TRAVIS_PULL_REQUEST" != "false" ]
		then
		errCause="Either one or more tests failed. Check the above logs"
		printExecLogs
        else
		errCause="Either one or more tests failed. Check the log file $EXECLOGS"
	fi
	checkError $retVal "Testrunner execution failed" "$errCause" "Run pxscene with testrunner.js locally as ./pxscene.sh https://px-apps.sys.comcast.net/pxscene-samples/examples/px-reference/test-run/testRunner.js?tests=<pxcore dir>tests/pxScene2d/testRunner/tests.json"
	exit 1;
fi

# Check for pxobject or texture memory leaks
grep "pxobjectcount is \[0\]" $EXECLOGS
pxRetVal=$?
grep "texture memory usage is \[0\]" $EXECLOGS
texRetVal=$?

if [[ $pxRetVal == 0 ]] && [[ $texRetVal == 0 ]] ; then
	printf "\nINFO: No pxObject leaks or Texture leaks found - GOOD ! \n"
else
	if [ "$TRAVIS_PULL_REQUEST" != "false" ]
		then
		errCause="Check the above logs"
		printExecLogs
	else
		errCause="Check the $EXECLOGS file"
	fi 
	checkError -1 "Texture leak or pxobject leak" "$errCause" "Follow steps locally: export PX_DUMP_MEMUSAGE=1;export RT_LOG_LEVEL=info;./pxscene.sh $TESTRUNNERURL?tests=<pxcore dir>/tests/pxScene2d/testRunner/tests.json locally and check for 'texture memory usage is' and 'pxobjectcount is' in logs and see which is non-zero" 
	exit 1;
fi

# Check for memory leaks
if [ "$leakcount" -ne 0 ]
	then
	if [ "$TRAVIS_PULL_REQUEST" != "false" ]
		then
		errCause="Check the above logs"
		printExecLogs
	else
		errCause="Check the file $LEAKLOGS and $EXECLOGS"
	fi
	checkError $leakcount "Execution reported memory leaks" "$errCause" "Run locally with these steps: export ENABLE_MEMLEAK_CHECK=1;export MallocStackLogging=1;export PX_DUMP_MEMUSAGE=1;./pxscene.sh $TESTRUNNERURL?tests=<pxcore dir>/tests/pxScene2d/testRunner/tests.json &; run leaks -nocontext pxscene >logfile continuously until the testrunner execution completes; Analyse the logfile" 
	exit 1;
else
	echo "Valgrind reports success !!!!!!!!!!!"
fi
exit 0;
