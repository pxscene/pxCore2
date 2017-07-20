#!/bin/sh

BUILDLOGS=$TRAVIS_BUILD_DIR/logs/build_logs

checkError()
{
  if [ "$1" -ne 0 ]
  then
  echo "Build failed with errors !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!";
  echo "CI failure reason: $3"
  echo "Cause: $4"
  echo "Reproduction/How to fix: $5"
  if [ "$2" -eq 1 ]
  then
  cat $BUILDLOGS
  fi
  exit 1;
  fi
}

if [ "$TRAVIS_EVENT_TYPE" != "cron" ] && [ "$TRAVIS_EVENT_TYPE" != "api" ] ;
then
export CODE_COVERAGE=1
fi

cd $TRAVIS_BUILD_DIR;
cp examples/pxScene2d/src/Makefile examples/pxScene2d/src/Makefile_orig

if [ "$TRAVIS_PULL_REQUEST" = "false" ]
then

echo "***************************** Generating config files ****" >> $BUILDLOGS
if [ "$TRAVIS_EVENT_TYPE" != "cron" ] && [ "$TRAVIS_EVENT_TYPE" != "api" ] ;
then
cmake -DBUILD_PX_TESTS=ON -DBUILD_PXSCENE_STATIC_LIB=ON . >>$BUILDLOGS 2>&1;
else
cmake . >>$BUILDLOGS 2>&1;
fi
checkError $? 0 "cmake config failed" "Config error" "Check the error in $BUILDLOGS"
echo "***************************** Building pxcore,rtcore,pxscene app,libpxscene,unitttests ****" >> $BUILDLOGS
cmake --build . >>$BUILDLOGS 2>&1;
checkError $? 0 "Building either pxcore,rtcore,pxscene app,libpxscene,unitttest failed" "Compilation error" "check the $BUILDLOGS file"

else

echo "***************************** Generating config files ****"
cmake -DBUILD_PX_TESTS=ON -DBUILD_PXSCENE_STATIC_LIB=ON . 1>>$BUILDLOGS;
checkError $? 1  "cmake config failed" "Config error" "Check the errors displayed in this window"
echo "***************************** Building pxcore,rtcore,pxscene app,libpxscene,unitttests ****" >> $BUILDLOGS
cmake --build . 1>>$BUILDLOGS;
checkError $? 1 "cmake build failed for pxcore or rtcore" "Compilation error" "Check the errors displayed in this window"
fi

if [ "$TRAVIS_PULL_REQUEST" = "false" ]
then
echo "***************************** Building pxscene deploy app ***" >> $BUILDLOGS

if [ "$TRAVIS_EVENT_TYPE" = "cron" ] || [ "$TRAVIS_EVENT_TYPE" = "api" ] ;
then

cd $TRAVIS_BUILD_DIR/examples/pxScene2d/src/

if [[ ! -z $PX_VERSION ]]
then
make -f Makefile_orig PXVERSION=$PX_VERSION deploy >>$BUILDLOGS 2>&1
checkError $? 0 "make command failed for deploy target" "Compilation error" "check the $BUILDLOGS file"

else

if [ "$TRAVIS_EVENT_TYPE" = "cron" ]
then
export linenumber=`awk '/CFBundleShortVersionString/{ print NR; exit }' $TRAVIS_BUILD_DIR/examples/pxScene2d/src/macstuff/Info.plist`
checkError $? 0 "unable to read string CFBundleShortVersionString from Info.plist file" "Parse error" "check whether the Info.plist file in pxscene repo is having CFBundleShortVersionString string or not?"
echo $linenumber
export PX_VERSION=`sed -n "\`echo $((linenumber+1))\`p" $TRAVIS_BUILD_DIR/examples/pxScene2d/src/macstuff/Info.plist|awk -F '<string>' '{print $2}'|awk -F'</string>' '{print $1}'`
checkError $? 0 "unable to read version from Info.plist file" "Parse error" "check whether the Info.plist file in pxscene repo is having version details or not?"
make -f Makefile_orig PXVERSION=$PX_VERSION deploy >>$BUILDLOGS 2>&1
checkError $? 0 "make command failed for deploy target" "Compilation error" "check the $BUILDLOGS file"

else
echo "Deploy terminated as pxversion environment is not set for api event type ************* " >> $BUILDLOGS
checkError 1 1 "Deploy terminated as pxversion environment is not set" "PX_VERSION environment variable not set" "Set PX_VERSION environment variable and retrigger"
fi

fi

fi

fi
cd $TRAVIS_BUILD_DIR
