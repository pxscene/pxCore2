#!/bin/sh

checkError()
{
  if [ "$1" -ne 0 ]
  then
    echo "script stage failed with errors !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
    echo "CI failure reason: $2"
    echo "Cause: $3"
    echo "Reproduction/How to fix: $4"
    exit 1
  fi
}

if [ "$TRAVIS_OS_NAME" = "linux" ]
then
    if [ "$TRAVIS_EVENT_TYPE" = "cron" ] || [ "$TRAVIS_EVENT_TYPE" = "api" ]
    then
      echo "Ignoring script stage for $TRAVIS_EVENT_TYPE event";
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
ulimit -c unlimited
export DUMP_STACK_ON_EXCEPTION=1
cd $TRAVIS_BUILD_DIR/ci
if [ "$TRAVIS_EVENT_TYPE" = "push" ] || [ "$TRAVIS_EVENT_TYPE" = "pull_request" ] ;
then
  sh build_px.sh "build_$TRAVIS_OS_NAME.sh" "testrunner_$TRAVIS_OS_NAME.sh" "unittests_$TRAVIS_OS_NAME.sh" "code_coverage_$TRAVIS_OS_NAME.sh" "pxleakcheck_$TRAVIS_OS_NAME.sh" "memleakdetector_$TRAVIS_OS_NAME.sh"
else
  sh build_px.sh "build_$TRAVIS_OS_NAME.sh"
fi
checkError $? "Build/unittests/testrunner/pxleakcheck/memleak detection failed" "Either build problem/execution problem" "Analyze corresponding log file"


if [ "$TRAVIS_EVENT_TYPE" = "cron" ] || [ "$TRAVIS_EVENT_TYPE" = "api" ] ;
then
  cp $TRAVIS_BUILD_DIR/examples/pxScene2d/src/deploy/mac/*.dmg $TRAVIS_BUILD_DIR/artifacts/.
  checkError $? "Copying dmg file failed" "Could be build problem or file not generated" "Analyze build logs"
  cp $TRAVIS_BUILD_DIR/examples/pxScene2d/src/deploy/mac/software_update.plist $TRAVIS_BUILD_DIR/artifacts/.
  checkError $? "Copying software_update.plist failed" "Could be build problem or file not generated" "Analyze build logs"
fi

exit 0;
