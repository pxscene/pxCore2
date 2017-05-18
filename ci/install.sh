#!/bin/sh

checkError()
{
  if [ "$1" -ne 0 ]
  then
    echo "install stage failed with errors !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
    exit 1
  fi
}

if [ "$TRAVIS_OS_NAME" = "linux" ]
then
    if [ "$TRAVIS_EVENT_TYPE" = "cron" ] || [ "$TRAVIS_EVENT_TYPE" = "api" ]
    then
      exit 0
    fi
fi

mkdir $TRAVIS_BUILD_DIR/logs
touch $TRAVIS_BUILD_DIR/logs/build_logs
BUILDLOGS=$TRAVIS_BUILD_DIR/logs/build_logs

if [ "$TRAVIS_EVENT_TYPE" = "push" ] || [ "$TRAVIS_EVENT_TYPE" = "pull_request" ] ;
then
  touch $TRAVIS_BUILD_DIR/logs/run_logs
  checkError $?
  mkdir $TRAVIS_BUILD_DIR/logs/codecoverage
  checkError $?
  touch $TRAVIS_BUILD_DIR/logs/pxcheck_logs
  checkError $?
fi

if [ "$TRAVIS_EVENT_TYPE" = "cron" ] || [ "$TRAVIS_EVENT_TYPE" = "api" ] ;
then
  mkdir $TRAVIS_BUILD_DIR/artifacts
  checkError $?
fi

echo "***************************** Building externals ****" > $BUILDLOGS
cd $TRAVIS_BUILD_DIR/examples/pxScene2d/external
./build.sh>>$BUILDLOGS
exit $?
