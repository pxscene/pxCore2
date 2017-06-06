#!/bin/sh

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

cd $TRAVIS_BUILD_DIR;
if [ "$TRAVIS_PULL_REQUEST" = "false" ]
then
echo "***************************** Building pxcore ****" >> $BUILDLOGS
xcodebuild clean
if [ "$TRAVIS_EVENT_TYPE" = "cron" ] || [ "$TRAVIS_EVENT_TYPE" = "api" ] ;
then
xcodebuild -scheme "pxCore Static Library" >>$BUILDLOGS 2>&1;
checkError $? 0
else
xcodebuild -scheme "pxCore Static Library" OTHER_CPLUSPLUSFLAGS="-fprofile-arcs -ftest-coverage">>$BUILDLOGS 2>&1;
checkError $? 0
fi
else
echo "***************************** Building pxcore ****"
xcodebuild clean
xcodebuild -scheme "pxCore Static Library" OTHER_CPLUSPLUSFLAGS="-fprofile-arcs -ftest-coverage" 1>>$BUILDLOGS;
checkError $? 1
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
checkError $? 0
else
if [ "$TRAVIS_EVENT_TYPE" = "cron" ]
then
export linenumber=`awk '/CFBundleShortVersionString/{ print NR; exit }' $TRAVIS_BUILD_DIR/examples/pxScene2d/src/macstuff/Info.plist`
checkError $? 0
echo $linenumber
export PX_VERSION=`sed -n "\`echo $((linenumber+1))\`p" $TRAVIS_BUILD_DIR/examples/pxScene2d/src/macstuff/Info.plist|awk -F '<string>' '{print $2}'|awk -F'</string>' '{print $1}'`
checkError $?
make PXVERSION=$PX_VERSION deploy >>$BUILDLOGS 2>&1
checkError $? 0
else
echo "Deploy terminated as pxversion environment is not set ************* " >> $BUILDLOGS
checkError 1 1
fi
fi
else
make -j CODE_COVERAGE=1 >>$BUILDLOGS 2>&1
checkError $? 0
fi
else
echo "***************************** Building pxscene app ***"
make -j CODE_COVERAGE=1 1>>$BUILDLOGS
checkError $? 0
fi

ls -lrt $TRAVIS_BUILD_DIR/examples/pxScene2d/src
cd $TRAVIS_BUILD_DIR/tests/pxScene2d;
if [ "$TRAVIS_PULL_REQUEST" = "false" ]
then
echo "***************************** Building unittests ***" >> $BUILDLOGS;
make clean;
make CODE_COVERAGE=1 >>$BUILDLOGS 2>&1;
checkError $? 0
else
echo "***************************** Building unittests ***";
make clean;
make CODE_COVERAGE=1 1>>$BUILDLOGS;
checkError $? 0
fi
