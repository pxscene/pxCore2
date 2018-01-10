#!/bin/sh

checkError()
{
  if [ "$1" -ne 0 ]
  then
    echo "install stage failed with errors !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
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
#check the PR file list, to check external is modified or not
fileList=$(git diff --name-only HEAD...$TRAVIS_BRANCH)
echo "***************File list*************************"
echo "$fileList"
echo "***************File list ends********************"
./ci/download_external.sh 96.116.56.119 "$TRAVIS_BUILD_DIR/examples/pxScene2d/">>$BUILDLOGS
if [ "$?" -eq 0 ]
then
  mv "$TRAVIS_BUILD_DIR/examples/pxScene2d/external" "$TRAVIS_BUILD_DIR/examples/pxScene2d/external_orig">> $BUILDLOGS
  tar xfz "$TRAVIS_BUILD_DIR/examples/pxScene2d/external.tgz $TRAVIS_BUILD_DIR/examples/pxScene2d/">> $BUILDLOGS
  if [ "$?" -eq 0 ]
  then 
    getPreBuiltExternal=true
  else
    mv "$TRAVIS_BUILD_DIR/examples/pxScene2d/external_orig" "$TRAVIS_BUILD_DIR/examples/pxScene2d/external" >> $BUILDLOGS 
  fi
else
  echo "********************External download Failed*****************">> $BUILDLOGS
  echo "********************External download Failed*****************"
fi


if [ "$getPreBuiltExternal" = true ]
then
  echo "*****************Pre-Built External available*****************">>$BUILDLOGS
  echo "*****************Pre-Built External available*****************"
else
  echo "***************************** Building externals ****" >> $BUILDLOGS
  echo "***************************** Building externals ****"
  cd $TRAVIS_BUILD_DIR/examples/pxScene2d/external
  ./build.sh>>$BUILDLOGS

  #Uploading the externals to server
  if [ "$?" -eq 0 ]
  then
    #if [ "$TRAVIS_OS_NAME" == "osx" ] && [ "TRAVIS_BRANCH" = "master" ] && [ "$TRAVIS_EVENT_TYPE" = "push" ]
    #then
      tar -czf $TRAVIS_BUILD_DIR/external.tar.gz $TRAVIS_BUILD_DIR/examples/pxScene2d/external/ >>$BUILDLOGS
      if [ "$?" -ne 0 ]
      then
        echo "***********Tar command failed****************">>$BUILDLOGS
        echo "***********Tar command failed****************"
      else
	ls -l |grep external
        cd $TRAVIS_BUILD_DIR
	./ci/deploy_external.sh 96.116.56.119 $TRAVIS_BUILD_DIR/external.tgz>>$BUILDLOGS
	if [ "$?" -ne 0 ]
	then
	  echo "***********Uploading of externals to the server failed****************">>$BUILDLOGS
	  echo "***********Uploading of externals to the server failed****************"
	fi	
	rm -f $TRAVIS_BUILD_DIR/external.tgz>>$BUILDLOGS
      fi
    #fi
  else
    checkError $? "building externals failed" "compilation error" "Need to build the externals directory locally in $TRAVIS_OS_NAME"
  fi
fi 


exit 0;
