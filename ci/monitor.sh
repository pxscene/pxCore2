#!/bin/sh
count=0
while [ "$count" -le 3600 ]; do
	sleep 60;
	count=$((count+60))
	echo "travis activities running for $count seconds"
        if [ "$count" -ge 800 ]; then
        top
        fi
done
