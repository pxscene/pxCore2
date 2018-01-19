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

if [ "$TRAVIS_OS_NAME" = "linux" ]
then
    if [ "$TRAVIS_EVENT_TYPE" = "cron" ] || [ "$TRAVIS_EVENT_TYPE" = "api" ]
    then
      echo "Ignoring install stage for $TRAVIS_EVENT_TYPE event";
      exit 0
    fi
fi

mkdir $TRAVIS_BUILD_DIR/logs
touch $TRAVIS_BUILD_DIR/logs/build_logs
BUILDLOGS=$TRAVIS_BUILD_DIR/logs/build_logs

if [ "$TRAVIS_EVENT_TYPE" = "push" ] || [ "$TRAVIS_EVENT_TYPE" = "pull_request" ] ;
then
  mkdir $TRAVIS_BUILD_DIR/logs/codecoverage
  checkError $? "unable to create codecoverage file" "could be permission issue" "Retry trigerring travis build"
  touch $TRAVIS_BUILD_DIR/logs/exec_logs
  checkError $? "unable to create exec logs file" "could be permission issue" "Retry trigerring travis build"
fi

if [ "$TRAVIS_EVENT_TYPE" = "cron" ] || [ "$TRAVIS_EVENT_TYPE" = "api" ] ;
then
  mkdir $TRAVIS_BUILD_DIR/artifacts
  checkError $? "unable to create directory artifacts" "could be permission issue" "Retry trigerring travis build"
fi

#before compiling check for stored externals
getPreBuiltExternal=false
cd $TRAVIS_BUILD_DIR

if [ "$TRAVIS_OS_NAME" = "osx" ] 
then
  #check the PR file list, to check external is modified or not
  fileList=$(git diff --name-only $TRAVIS_COMMIT_RANGE)
  echo "************************* File list *************************">>$BUILDLOGS
  echo $fileList >>$BUILDLOGS
  echo "********************** File list ends ***********************">>$BUILDLOGS
  if  echo $fileList | grep -q "pxScene2d/external/"; 
  then
    echo "***************** Externals are modified ******************">>$BUILDLOGS
  else
    echo "**************** Externals are not modified ***************">>$BUILDLOGS
    ./ci/download_external.sh 96.116.56.119 "$TRAVIS_BUILD_DIR/examples/pxScene2d/">>$BUILDLOGS
    if [ "$?" -eq 0 ]
    then
      echo "************** External download completed **************">> $BUILDLOGS
      getPreBuiltExternal=true
    else
      echo "*************** External download Failed ****************">> $BUILDLOGS
    fi
  fi
fi

if [ "$getPreBuiltExternal" = true ]
then
  echo "*************** Pre-Built External available ****************">>$BUILDLOGS
else
  echo "************* No Pre-Built External available ***************">>$BUILDLOGS
  echo "******************** Building externals *********************" >> $BUILDLOGS
  cd $TRAVIS_BUILD_DIR/examples/pxScene2d/external
  ./build.sh>>$BUILDLOGS

  #Uploading the externals to server
  if [ "$?" -eq 0 ]
  then
    #if [ "$TRAVIS_OS_NAME" = "osx" ] && [ "TRAVIS_BRANCH" = "master" ] && [ "$TRAVIS_EVENT_TYPE" = "push" ]
    if [ "$TRAVIS_OS_NAME" = "osx" ] && [ "$TRAVIS_EVENT_TYPE" = "push" ]
    then
      cd $TRAVIS_BUILD_DIR/examples/pxScene2d
      echo "tar -czf $TRAVIS_BUILD_DIR/external.tgz external" >>$BUILDLOGS
      tar -czf "$TRAVIS_BUILD_DIR/external_${TRAVIS_COMMIT}_${TRAVIS_BRANCH}.tgz" external >>$BUILDLOGS
      if [ "$?" -ne 0 ]
      then
        echo "************* Tar command failed *************">>$BUILDLOGS
      else
        cd $TRAVIS_BUILD_DIR
        ./ci/deploy_external.sh 96.116.56.119 "$TRAVIS_BUILD_DIR/external_${TRAVIS_COMMIT}_${TRAVIS_BRANCH}.tgz">>$BUILDLOGS
        if [ "$?" -ne 0 ]
        then
	  echo "********* Uploading of externals to the server failed *********">>$BUILDLOGS
        fi	
        rm -f "$TRAVIS_BUILD_DIR/external_${TRAVIS_COMMIT}_${TRAVIS_BRANCH}.tgz">>$BUILDLOGS
      fi
    fi
  else
    checkError $? "building externals failed" "compilation error" "Need to build the externals directory locally in $TRAVIS_OS_NAME" >>$BUILDLOGS
  fi
fi 

exit 0;
