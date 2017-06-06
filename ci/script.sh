#!/bin/sh

checkError()
{
  if [ "$1" -ne 0 ]
  then
    echo "script stage failed with errors !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
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


if [ "$TRAVIS_EVENT_TYPE" = "push" ] || [ "$TRAVIS_EVENT_TYPE" = "pull_request" ] ;
then
  if [ "$TRAVIS_OS_NAME" = "linux" ]; 
  then 
    export DISPLAY=:99.0
    sh -e /etc/init.d/xvfb start
    sleep 3
  fi
fi

retval=0
cd $TRAVIS_BUILD_DIR/ci
if [ "$TRAVIS_EVENT_TYPE" = "push" ] || [ "$TRAVIS_EVENT_TYPE" = "pull_request" ] ;
then
  ps -aef|grep  lighttpd
  netstat -a
  sh build_px.sh "build_$TRAVIS_OS_NAME.sh" "testrunner_$TRAVIS_OS_NAME.sh" "unittests_$TRAVIS_OS_NAME.sh" "code_coverage_$TRAVIS_OS_NAME.sh" "pxleakcheck_$TRAVIS_OS_NAME.sh" "memleakdetector_$TRAVIS_OS_NAME.sh"
else
  sh build_px.sh "build_$TRAVIS_OS_NAME.sh"
fi
checkError $?


if [ "$TRAVIS_EVENT_TYPE" = "cron" ] || [ "$TRAVIS_EVENT_TYPE" = "api" ] ;
then
  cp $TRAVIS_BUILD_DIR/examples/pxScene2d/src/deploy/mac/*.dmg $TRAVIS_BUILD_DIR/artifacts/.
  checkError $?
  cp $TRAVIS_BUILD_DIR/examples/pxScene2d/src/deploy/mac/software_update.plist $TRAVIS_BUILD_DIR/artifacts/.
  checkError $?
fi

exit 0;
