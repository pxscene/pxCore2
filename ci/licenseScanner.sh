#!/bin/bash

if [ -z "${TRAVIS_BUILD_DIR}" ]
then
  printf "\nFATAL ERROR:  'TRAVIS_BUILD_DIR' env var is NOT defined\n\n"
  exit 1;
else
  printf "\nBase Dir: ${TRAVIS_BUILD_DIR}\n\n"
fi


LICENSE_STR="Licensed under the Apache License, Version 2.0"
LICENSE_STR2="Licensed under the"
COPY_STR="Copyright (C)"

scanResult="PASS"
fileCount=0

dirs=("$TRAVIS_BUILD_DIR/src" "$TRAVIS_BUILD_DIR/remote" "$TRAVIS_BUILD_DIR/tests" "$TRAVIS_BUILD_DIR/examples/pxScene2d/src/")

printf "****************************** Message Start ******************************\n"
printf "\nDirectories included for scanning : \n"
for dir in ${dirs[@]} 
do 
printf "$dir\n" 
done 
printf "\nFiles extensions included for scanning : C, CPP, H \n\n"
printf "****************************** Message Ends *******************************\n"

for dir in ${dirs[@]}
do
  printf "\n\nProcessing $dir ...\n"
  printf "=====================================================\n"
  files=(`find $dir -regex ".*\.\(c\|cpp\|h\)"`)
  for file in ${files[@]}
  do
    #printf "Scanning $file \n"
    grep  -q "$LICENSE_STR" $file
    if [ "$?" != "0" ] ; then
      #check for any other license availability
      grep -q -e "$COPY_STR" -e "$LICENSE_STR2" $file
      if [ "$?" == "0" ] ; then
        printf "[EXCLUDED] Different license available : $file !!!\n"
        continue
      else
        fileCount=$((fileCount + 1))
        scanResult="FAIL"
        printf "[FAILED] License Not Found : $file !!!\n"
      fi
      #if [ "$1" == "ON" ] ; then 
        #cp "$TRAVIS_BUILD_DIR/ci/apache_license.txt" $file.tmp
        #cat $file >> $file.tmp
        #mv $file.tmp $file
      #fi
    fi 
  done
done

if [  "$scanResult" == "FAIL" ] ; then
  printf "\n [ERROR] $fileCount Files without license are detected. \n"
  exit 1
else
  printf "\n [SUCCESS] File scan completed successfully. \n"
  exit 1
fi
