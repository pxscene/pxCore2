#!/bin/bash

if [ -z "${TRAVIS_BUILD_DIR}" ]
then
  printf "\nFATAL ERROR:  'TRAVIS_BUILD_DIR' env var is NOT defined\n\n"
  exit 1;
else
  printf "\nBase Dir: ${TRAVIS_BUILD_DIR}\n\n"
fi

echo ">>>>>>>>>>>>..$1"

export LICENSE_STR="Licensed under the Apache License, Version 2.0"
export LICENSE_STR2="http://www.apache.org/licenses/LICENSE-2.0"
export LICENSE_STR3="John Robinson"
export LICENSE_STR4="2005-2006"

scanResult="PASS"

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
  fileCount=0
  for file in ${files[@]}
  do
    #printf "Scanning $file \n"
    grep  -q "$LICENSE_STR" $file
    #grep -q "$LICENSE_STR3" $file
    if [ "$?" != "0" ] ; then
      fileCount=$((fileCount + 1))
      scanResult="FAIL"
      printf "[$fileCount/${#files[@]}] License Not Found : $file !!!\n"
      if [ "$1" == "ON" ] ; then 
        #cp "$TRAVIS_BUILD_DIR/ci/apache_license.txt" $file.tmp
        #cat $file >> $file.tmp
        #mv $file.tmp $file
      fi
    fi 
  done
done

if [  "$scanResult" == "FAIL" ] ; then
  printf "\n Files without license are detected \n"
  exit 1
fi
