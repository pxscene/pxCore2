#!/bin/sh

BUILDLOGS=$TRAVIS_BUILD_DIR/logs/build_logs

mkdir -p $TRAVIS_BUILD_DIR/logs   # make 'logs' if necessary

if [[ ! -z $PX_VERSION ]]
then
  export PX_BUILD_VERSION=$PX_VERSION
fi

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
checkError()
{
if [ "$1" -ne 0 ]
then
    printf "\n\n*********************************************************************";
    printf "\n********************* BUILD FAIL DETAILS ****************************";
    printf "\nCI failure reason: $3"
    printf "\nCause: $4"
    printf "\nReproduction/How to fix: $5"
    printf "\n*********************************************************************";
    printf "\n*********************************************************************\n\n";
	if [ "$2" -eq 1 ]
    then
        printf "\n\n************************ PRINTING BUILD LOGS *************************";
        cat $BUILDLOGS
        printf "\n**********************************************************************\n\n";
    fi
exit 1;
fi
}
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

if [ "$TRAVIS_EVENT_TYPE" != "cron" ] && [ "$TRAVIS_EVENT_TYPE" != "api" ] && [ -z "${TRAVIS_TAG}" ] 
then
  export CODE_COVERAGE=1
  export USE_HTTP_CACHE=1
fi
cd $TRAVIS_BUILD_DIR;
mkdir -p temp
cd  temp

if [ "$TRAVIS_PULL_REQUEST" = "false" ]
then
  echo "***************************** Generating config files ****" >> $BUILDLOGS
  if [ "$TRAVIS_EVENT_TYPE" != "cron" ] && [ "$TRAVIS_EVENT_TYPE" != "api" ] && [ -z "${TRAVIS_TAG}" ] 
  then
    cmake -DBUILD_PX_TESTS=ON -DBUILD_PXSCENE_STATIC_LIB=ON -DBUILD_DEBUG_METRICS=ON -DPXSCENE_TEST_HTTP_CACHE=ON -DENABLE_THREAD_SANITIZER=ON .. >>$BUILDLOGS 2>&1;
  else
    if [ "$TRAVIS_EVENT_TYPE" == "cron" ] ; 
    then
      cp ../examples/pxScene2d/src/macstuff/Resources/SparkEdge.icns ../examples/pxScene2d/src/macstuff/Resources/pxscene.icns
      cp ../examples/pxScene2d/src/macstuff/Resources/SparkEdge.icns ../examples/pxScene2d/src/macstuff/Resources/AppIcon.icns
      cp ../examples/pxScene2d/src/macstuff/Resources/SparkEdge.icns ../examples/pxScene2d/src/macstuff/dmgresources/pxscene.icns
      cp ../examples/pxScene2d/src/browser/images/status_bg_edge.svg ../examples/pxScene2d/src/browser/images/status_bg.svg
       
      cmake -DSUPPORT_DUKTAPE=OFF -DPXSCENE_VERSION=edge_`date +%Y-%m-%d` .. >>$BUILDLOGS 2>&1;
    else
      cmake -DSUPPORT_DUKTAPE=OFF .. >>$BUILDLOGS 2>&1;
    fi
  fi

  checkError $? 0 "cmake config failed" "Config error" "Check the error in $BUILDLOGS"

  echo "***************************** Building pxcore,rtcore,pxscene app,libpxscene,unitttests ****" >> $BUILDLOGS
  cmake --build . -- -j$(getconf _NPROCESSORS_ONLN) >>$BUILDLOGS 2>&1;
  checkError $? 0 "Building either pxcore,rtcore,pxscene app,libpxscene,unitttest failed" "Compilation error" "check the $BUILDLOGS file"

else

  echo "***************************** Generating config files ****"
  cmake -DBUILD_PX_TESTS=ON -DBUILD_PXSCENE_STATIC_LIB=ON -DBUILD_DEBUG_METRICS=OFF -DPXSCENE_TEST_HTTP_CACHE=ON -DENABLE_THREAD_SANITIZER=ON .. 1>>$BUILDLOGS;
  checkError $? 1  "cmake config failed" "Config error" "Check the errors displayed in this window"

  echo "***************************** Building pxcore,rtcore,pxscene app,libpxscene,unitttests ****" >> $BUILDLOGS
  cmake --build . -- -j$(getconf _NPROCESSORS_ONLN) 1>>$BUILDLOGS;
  checkError $? 1 "Building either pxcore,rtcore,pxscene app,libpxscene,unitttest failed" "Compilation error" "Check the errors displayed in this window"
fi

cd $TRAVIS_BUILD_DIR
if [ "$TRAVIS_PULL_REQUEST" = "false" ]
  then
  echo "*********************** Building pxscene deploy app ***********************" >> $BUILDLOGS

  if [ "$TRAVIS_EVENT_TYPE" = "cron" ]  ;
  then
    cd $TRAVIS_BUILD_DIR/examples/pxScene2d/src/
    ./mkdeploy.sh edge_`date +%Y-%m-%d` >>$BUILDLOGS 2>&1
        
  elif [ "$TRAVIS_EVENT_TYPE" = "api" ] || [ ! -z "${TRAVIS_TAG}" ] 
  then
    cd $TRAVIS_BUILD_DIR/examples/pxScene2d/src/

    if [[ ! -z $PX_VERSION ]]
    then
      #make PXVERSION=$PX_VERSION deploy >>$BUILDLOGS 2>&1
      #checkError $? 0 "make command failed for deploy target" "Compilation error" "check the $BUILDLOGS file"
      echo "built with cmake"
     ./mkdeploy.sh $PX_VERSION >>$BUILDLOGS 2>&1
    else
      export linenumber=`awk '/CFBundleShortVersionString/{ print NR; exit }' $TRAVIS_BUILD_DIR/examples/pxScene2d/src/macstuff/Info.plist`
      checkError $? 0 "unable to read string CFBundleShortVersionString from Info.plist file" "Parse error" "check whether the Info.plist file in pxscene repo is having CFBundleShortVersionString string or not?"
      echo $linenumber
      export PX_VERSION=`sed -n "\`echo $((linenumber+1))\`p" $TRAVIS_BUILD_DIR/examples/pxScene2d/src/macstuff/Info.plist|awk -F '<string>' '{print $2}'|awk -F'</string>' '{print $1}'`
      checkError $? 0 "unable to read version from Info.plist file" "Parse error" "check whether the Info.plist file in pxscene repo is having version details or not?"
      #make PXVERSION=$PX_VERSION deploy >>$BUILDLOGS 2>&1
      ./mkdeploy.sh $PX_VERSION >>$BUILDLOGS 2>&1
      checkError $? 0 "make command failed for deploy target" "Compilation error" "check the $BUILDLOGS file"
    fi
  fi
#else
#echo "Deploy terminated as pxversion environment is not set for api event type ************* " >> $BUILDLOGS
#checkError 1 1 "Deploy terminated as pxversion environment is not set" "PX_VERSION environment variable not set" "Set PX_VERSION environment variable and retrigger"
fi



cd $TRAVIS_BUILD_DIR
exit 0;
