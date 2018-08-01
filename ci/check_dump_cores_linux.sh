#!/bin/bash
#script to check for corefile and get the corestack reported

path="$1"
binary="$2"
processId="$3"
logfile="$4"
cd $path
ls -lrt /tmp/pxscenecrash
retVal=$?
if [ "$retVal" -eq 0 ]
	then
        sudo gdb -q --command="$TRAVIS_BUILD_DIR/ci/debuggercmds_linux" --pid="$processId" "$binary" 2&> gdblogs
        if [ "$TRAVIS_PULL_REQUEST" != "false" ]
		then
		echo "**********************PRINTING CORE STACK DETAILS**************************"
		cat gdblogs
		echo "***************************************************************************"
	else
		echo "**********************PRINTING CORE STACK DETAILS**************************" >> $logfile 
		cat gdblogs  >> $logfile
		echo "***************************************************************************" >> $logfile
	fi
	rm -rf gdblogs
	exit 1;
else
	echo "No core files generated in $path"
fi
exit 0;
