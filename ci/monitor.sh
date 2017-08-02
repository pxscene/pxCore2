#!/bin/sh
count=0
while [ "$count" -ne 3600 ]; do
sleep 60;
count=$((count+60))
echo "travis activities running for $count seconds"
done
