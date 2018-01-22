#!/bin/sh

checkError()
{
  if [ "$1" -ne 0 ]
  then
    printf "\n\n*********************************************************************";
    printf "\n******************** SCRIPT FAIL DETAILS ****************************";
    printf "\nCI failure reason: $2"
    printf "\nCause: $3"
    printf "\nReproduction/How to fix: $4"
    printf "\n*********************************************************************";
    printf "\n*********************************************************************\n\n";
    exit 1
  fi
}

mkdir $TRAVIS_BUILD_DIR/logs
touch $TRAVIS_BUILD_DIR/logs/build_logs
BUILDLOGS=$TRAVIS_BUILD_DIR/logs/build_logs

echo "************************ Building externals *************************"
cd $TRAVIS_BUILD_DIR/examples/pxScene2d/external
./build.sh>>$BUILDLOGS
checkError $? "building externals failed" "compilation error" "Need to build the externals directory locally in $TRAVIS_OS_NAME"
exit 0;
