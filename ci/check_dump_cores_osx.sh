#!/bin/bash
#script to check for corefile and get the corestack reported

path="$1"
process="$2"
logfile="$3"

#take report from lldb

cd $path
echo "sudo lldb -o \"attach $process\" -o \"bt all\" -o \"quit\" 1>lldblogs 2>/dev/null"
sudo lldb -o "attach $process" -o "bt all" -o "quit" 1>lldblogs 2>/dev/null

if [ "$TRAVIS_PULL_REQUEST" != "false" ]
	then
	echo "********************PRINTING CORE STACK DETAILS************************"
        echo "PATH :  $path"
	sudo cat lldblogs
	echo "***********************************************************************"
else
	echo "********************PRINTING CORE STACK DETAILS************************" >> $logfile
	echo "PATH :  $path" >> $logfile
	sudo cat lldblogs >> $logfile
	echo "***********************************************************************"
fi
sudo rm -rf lldblogs
exit 0;
