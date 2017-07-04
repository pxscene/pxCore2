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

cd $TRAVIS_BUILD_DIR;
if [ "$TRAVIS_PULL_REQUEST" = "false" ]
then
echo "***************************** Building pxcore ****" >> $BUILDLOGS
xcodebuild clean
if [ "$TRAVIS_EVENT_TYPE" = "cron" ] || [ "$TRAVIS_EVENT_TYPE" = "api" ] ;
then
xcodebuild -scheme "pxCore Static Library" >>$BUILDLOGS 2>&1;
checkError $? 0 "Building pxcore static library failed" "Compilation error" "check the $BUILDLOGS file"
else
xcodebuild -scheme "pxCore Static Library" OTHER_CPLUSPLUSFLAGS="-fprofile-arcs -ftest-coverage">>$BUILDLOGS 2>&1;
checkError $? 0 "Building pxcore static library failed" "Compilation error" "check the $BUILDLOGS file"
fi
else
echo "***************************** Building pxcore ****"
xcodebuild clean
xcodebuild -scheme "pxCore Static Library" OTHER_CPLUSPLUSFLAGS="-fprofile-arcs -ftest-coverage" 1>>$BUILDLOGS;
checkError $? 1 "Building pxcore static library failed" "Compilation error" "check the logs displayed in this window"
fi
cd $TRAVIS_BUILD_DIR/examples/pxScene2d/src;

if [ "$TRAVIS_PULL_REQUEST" = "false" ]
then
echo "***************************** Building libpxscene ****" >> $BUILDLOGS;
make clean;
make libs-mac CODE_COVERAGE=1 >>$BUILDLOGS 2>&1;
checkError $? 0
else
echo "***************************** Building libpxscene ****";
make clean;
make libs-mac CODE_COVERAGE=1 1>>$BUILDLOGS;
checkError $? 0
fi
if [ "$TRAVIS_PULL_REQUEST" = "false" ]
then
echo "***************************** Building pxscene app ***" >> $BUILDLOGS
if [ "$TRAVIS_EVENT_TYPE" = "cron" ] || [ "$TRAVIS_EVENT_TYPE" = "api" ] ;
then
if [[ ! -z $PX_VERSION ]]
then
make PXVERSION=$PX_VERSION deploy >>$BUILDLOGS 2>&1
checkError $? 0 "make command failed for deploy target" "Compilation error" "check the $BUILDLOGS file"
else
if [ "$TRAVIS_EVENT_TYPE" = "cron" ]
then
export linenumber=`awk '/CFBundleShortVersionString/{ print NR; exit }' $TRAVIS_BUILD_DIR/examples/pxScene2d/src/macstuff/Info.plist`
checkError $? 0 "unable to read string CFBundleShortVersionString from Info.plist file" "Parse error" "check whether the Info.plist file in pxscene repo is having CFBundleShortVersionString string or not?"
echo $linenumber
export PX_VERSION=`sed -n "\`echo $((linenumber+1))\`p" $TRAVIS_BUILD_DIR/examples/pxScene2d/src/macstuff/Info.plist|awk -F '<string>' '{print $2}'|awk -F'</string>' '{print $1}'`
checkError $? 0 "unable to read version from Info.plist file" "Parse error" "check whether the Info.plist file in pxscene repo is having version details or not?"
make PXVERSION=$PX_VERSION deploy >>$BUILDLOGS 2>&1
checkError $? 0 "make command failed for deploy target" "Compilation error" "check the $BUILDLOGS file"
else
echo "Deploy terminated as pxversion environment is not set ************* " >> $BUILDLOGS
checkError 1 1 "Deploy terminated as pxversion environment is not set" "PX_VERSION environment variable not set" "Set PX_VERSION environment variable and retrigger"
fi
fi
else
make -j CODE_COVERAGE=1 >>$BUILDLOGS 2>&1
checkError $? 0 "make command failed for pxscene target" "Compilation error" "check the $BUILDLOGS file"
fi
else
echo "***************************** Building pxscene app ***"
make clean;
make -j CODE_COVERAGE=1 1>>$BUILDLOGS
checkError $? 0 "make command failed for pxscene target" "Compilation error" "check the errors displayed in this window"
fi
