#!/bin/sh

EXECLOGS=$TRAVIS_BUILD_DIR/logs/exec_logs

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
printExecLogs()
{
  printf "\n\n********************** PRINTING EXEC LOG **************************\n\n"
  cat $EXECLOGS
  printf "\n\n**********************     LOG ENDS      **************************\n\n"
}
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

checkError()
{
  if [ "$1" -ne 0 ]
  then
    printf "\n\n*********************************************************************";
    printf "\n********************* SCRIPT FAIL DETAILS *****************************";
    printf "\nCI failure reason: $2"
    printf "\nCause: $3"
    printf "\nReproduction/How to fix: $4"
    printf "\n*********************************************************************";
    printf "\n*********************************************************************\n\n";
    printExecLogs
    exit 1
  fi
}

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

if [ "$TRAVIS_OS_NAME" = "linux" ]
then
  if  [ "$TRAVIS_EVENT_TYPE" = "api" ] || [ ! -z "${TRAVIS_TAG}" ]
    then
    echo "Ignoring script stage for $TRAVIS_EVENT_TYPE event";
    exit 0
  fi
  if [ "$TRAVIS_EVENT_TYPE" = "cron" ] 
  then
    # Since We don't have any plans to include permission file to appveyor, uploading of artifacts to build server directly from appveyor cannot be done.
    # So it is planned to fetch the artifact from appveyor in travis builds and to upload it to build server.
    # By this, the below script is called during the travis cron job, and it will upload the latest available artifact to build server.
    ./ci/uploadWindowsArtifact.sh
    checkError $? "./ci/uploadWindowsArtifact.sh Failed" "Artifact not available or Failed to upload" "Verify ./ci/uploadWindowsArtifact.sh locally"
    exit 0
  fi
fi


if ( [ "$TRAVIS_EVENT_TYPE" = "push" ] || [ "$TRAVIS_EVENT_TYPE" = "pull_request" ] ) && [ -z "${TRAVIS_TAG}" ] 
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
if ( [ "$TRAVIS_EVENT_TYPE" = "push" ] || [ "$TRAVIS_EVENT_TYPE" = "pull_request" ] ) && [ -z "${TRAVIS_TAG}" ] 
then
  sh build_px.sh 
  checkError $? "#### Build/unittests/execution [build_px.sh] failed" "Either build problem/execution problem" "Analyze corresponding log file"

  sh "build_$TRAVIS_OS_NAME.sh" 
  checkError $? "#### Build/unittests/execution [build_$TRAVIS_OS_NAME.sh] failed" "Either build problem/execution problem" "Analyze corresponding log file"

  sh "unittests_$TRAVIS_OS_NAME.sh" 
  checkError $? "#### Build/unittests/execution [unittests_$TRAVIS_OS_NAME.sh] failed" "Either build problem/execution problem" "Analyze corresponding log file"

  sh "execute_$TRAVIS_OS_NAME.sh" 
  checkError $? "#### Build/unittests/execution [execute_$TRAVIS_OS_NAME.sh] failed" "Either build problem/execution problem" "Analyze corresponding log file"

  sh "code_coverage_$TRAVIS_OS_NAME.sh"
  checkError $? "#### Build/unittests/execution [code_coverage_$TRAVIS_OS_NAME.sh] failed" "Either build problem/execution problem" "Analyze corresponding log file"

else
  sh build_px.sh "build_$TRAVIS_OS_NAME.sh"
fi

if [ "$TRAVIS_EVENT_TYPE" = "api" ] || [ ! -z "${TRAVIS_TAG}" ] 
then
  cp $TRAVIS_BUILD_DIR/examples/pxScene2d/src/deploy/mac/*.dmg $TRAVIS_BUILD_DIR/artifacts/.
  checkError $? "Copying dmg file failed" "Could be build problem or file not generated" "Analyze build logs"
  cp $TRAVIS_BUILD_DIR/examples/pxScene2d/src/deploy/mac/software_update.plist $TRAVIS_BUILD_DIR/artifacts/.
  checkError $? "Copying software_update.plist failed" "Could be build problem or file not generated" "Analyze build logs"
fi

if [ "$TRAVIS_EVENT_TYPE" = "cron" ] ;
then
  cp $TRAVIS_BUILD_DIR/examples/pxScene2d/src/deploy/mac/SparkEdge.dmg $TRAVIS_BUILD_DIR/artifacts/SparkEdge.dmg
  checkError $? "Copying dmg file failed" "Could be build problem or file not generated" "Analyze build logs"
  cp $TRAVIS_BUILD_DIR/examples/pxScene2d/src/deploy/mac/software_update.plist $TRAVIS_BUILD_DIR/artifacts/.
  checkError $? "Copying software_update.plist failed" "Could be build problem or file not generated" "Analyze build logs"
fi

exit 0;
