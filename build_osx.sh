#!/bin/sh
# 
# TODO: Add more comments
#

BUILDLOGS=$TRAVIS_BUILD_DIR/logs/build_logs
checkError()
{
  if [ "$1" -ne 0 ]
  then
  echo "Build failed with errors !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!";
  if [ "$2" -eq 1 ]
  then
  cat $BUILDLOGS
  fi
  exit 1;
  fi
}
cd $TRAVIS_BUILD_DIR/src
cd $TRAVIS_BUILD_DIR;
if [ "$TRAVIS_PULL_REQUEST" = "false" ]
then
echo "***************************** Building pxcore ****" >> $BUILDLOGS
xcodebuild clean
xcodebuild -scheme "pxCore Static Library">>$BUILDLOGS 2>&1;
checkError $? 0
else
echo "***************************** Building pxcore ****"
xcodebuild clean
xcodebuild -scheme "pxCore Static Library" 1>>$BUILDLOGS;
checkError $? 1
fi
cd $TRAVIS_BUILD_DIR/examples/pxScene2d/src;

if [ "$TRAVIS_PULL_REQUEST" = "false" ]
then
echo "***************************** Building pxscene app ***" >> $BUILDLOGS
make clean;
make -j>>$BUILDLOGS 2>&1
checkError $? 0
else
echo "***************************** Building pxscene app ***"
make clean;
make -j 1>>$BUILDLOGS
checkError $? 0
fi
