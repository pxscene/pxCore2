#!/bin/sh
BUILDLOGS=$TRAVIS_BUILD_DIR/logs/build_logs
checkError()
{
  if [ "$1" -ne 0 ]
  then
  echo "Build failed with errors !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!";
  echo "CI failure reason: $2"
  echo "Cause: $3"
  echo "Reproduction/How to fix: $4"
  exit 1;
  fi
}

export CODE_COVERAGE=1
cd $TRAVIS_BUILD_DIR

if [ "$TRAVIS_PULL_REQUEST" = "false" ]
then
echo "***************************** Generating config files ****" >> $BUILDLOGS
cmake -DBUILD_PX_TESTS=ON -DBUILD_PXSCENE_STATIC_LIB=ON . >>$BUILDLOGS 2>&1;
checkError $? "cmake config failed" "Config error" "Check the error in $BUILDLOGS"

echo "***************************** Building pxcore and rtcore ****" >> $BUILDLOGS
cmake --build src/ >>$BUILDLOGS 2>&1;
checkError $? "cmake build failed for pxcore or rtcore" "Compilation error" "Check the error in $BUILDLOGS"

echo "***************************** Building pxscene app and libpxscene ****" >> $BUILDLOGS;
cmake --build examples/pxScene2d/src/ >>$BUILDLOGS 2>&1;
checkError $? "cmake build failed or pxscene app or libpxscene" "Compilation error" "Check the error in $BUILDLOGS"

echo "***************************** Building unittests ***" >> $BUILDLOGS;
cmake --build tests/pxScene2d/ >>$BUILDLOGS 2>&1;
checkError $? "cmake build failed for unittests" "Compilation error" "Check the error in $BUILDLOGS"

else
echo "***************************** Generating config files ****"
cmake -DBUILD_PX_TESTS=ON -DBUILD_PXSCENE_STATIC_LIB=ON . 1>>$BUILDLOGS;
checkError $? "cmake config failed" "Config error" "Check the errors displayed in this window"

echo "***************************** Building pxcore and rtcore ****"
pwd
ls -rlt
ls -rlt src/
ls -rlt examples/pxScene2d/src/
ls -lrt tests/pxScene2d/

cmake --build src/ 1>>$BUILDLOGS;
checkError $? "cmake build failed for pxcore or rtcore" "Compilation error" "Check the errors displayed in this window"

echo "***************************** Building pxscene app and libpxscene ****";
cmake --build examples/pxScene2d/src/ 1>>$BUILDLOGS;
checkError $? "cmake build failed or pxscene app or libpxscene" "Compilation error" "Check the errors displayed in this window"

echo "***************************** Building unittests ***";
cmake --build tests/pxScene2d/ 1>>$BUILDLOGS;
checkError $? "cmake build failed for unittests" "Compilation error" "Check the errors displayed in this window"
fi
