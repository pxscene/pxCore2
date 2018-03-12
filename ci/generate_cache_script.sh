#!/bin/sh

checkError()
{
  if [ "$1" -ne 0 ]
  then
    echo "*********************************************************************";
    echo "*********************SCRIPT FAIL DETAILS*****************************";
    echo "CI failure reason: $2"
    echo "Cause: $3"
    echo "Reproduction/How to fix: $4"
    echo "*********************************************************************";
    echo "*********************************************************************";
    exit 1
  fi
}

mkdir $TRAVIS_BUILD_DIR/logs
touch $TRAVIS_BUILD_DIR/logs/build_logs
BUILDLOGS=$TRAVIS_BUILD_DIR/logs/build_logs

echo "***************************** Building externals ****" > $BUILDLOGS
cd $TRAVIS_BUILD_DIR/examples/pxScene2d/external
./build.sh>>$BUILDLOGS
checkError $? "building externals failed" "compilation error" "Need to build the externals directory locally in $TRAVIS_OS_NAME"
exit 0;
