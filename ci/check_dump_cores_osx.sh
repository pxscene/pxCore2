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
	echo "Core stack details ---------------------------------------- from $path"
	sudo cat lldblogs
else
	echo "Core stack details ---------------------------------------- from $path" >> $logfile
	sudo cat lldblogs >> $logfile
fi
sudo rm -rf lldblogs
exit 0;
