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

cd $TRAVIS_BUILD_DIR/src
if [ "$TRAVIS_PULL_REQUEST" = "false" ]
then
echo "***************************** Building pxcore and rtcore ****" >> $BUILDLOGS
make -f Makefile.glut all CODE_COVERAGE=1 >>$BUILDLOGS 2>&1;
checkError $? "Makefile.glut build failed for target all" "Compilation error" "Check the error in $BUILDLOGS"
make -f Makefile.glut rtcore CODE_COVERAGE=1 >>$BUILDLOGS 2>&1;
checkError $? "Makefile.glut build failed for target rtcore" "Compilation error" "Check the error in $BUILDLOGS"
else
echo "***************************** Building pxcore and rtcore ****"
make -f Makefile.glut all CODE_COVERAGE=1 1>>$BUILDLOGS;
checkError $? "Makefile.glut build failed for target all" "Compilation error" "Check the errors displayed in this window"
make -f Makefile.glut rtcore CODE_COVERAGE=1 1>>$BUILDLOGS;
checkError $? "Makefile.glut build failed for target rtcore" "Compilation error" "Check the errors displayed in this window"
fi

cd $TRAVIS_BUILD_DIR/examples/pxScene2d/src
if [ "$TRAVIS_PULL_REQUEST" = "false" ]
then
echo "***************************** Building libpxscene ****" >> $BUILDLOGS;
make clean;
make libs-glut CODE_COVERAGE=1 >>$BUILDLOGS 2>&1;
checkError $? "Makefile build failed for target libs-glut" "Compilation error" "Check the error in $BUILDLOGS"
else
echo "***************************** Building libpxscene ****";
make clean;
make libs-glut CODE_COVERAGE=1 1>>$BUILDLOGS;
checkError $? "Makefile build failed for target libs-glut" "Compilation error" "Check the errors displayed in this window"
fi

if [ "$TRAVIS_PULL_REQUEST" = "false" ]
then
echo "***************************** Building pxscene app ***" >> $BUILDLOGS
make -j CODE_COVERAGE=1 >>$BUILDLOGS 2>&1
checkError $? "Makefile build failed for target pxscene app" "Compilation error" "Check the error in $BUILDLOGS"
else
echo "***************************** Building pxscene app ***"
make -j CODE_COVERAGE=1 1>>$BUILDLOGS
checkError $? "Makefile build failed for target pxscene app" "Compilation error" "Check the errors displayed in this window"
fi

cd $TRAVIS_BUILD_DIR/tests/pxScene2d;
if [ "$TRAVIS_PULL_REQUEST" = "false" ]
then
echo "***************************** Building unittests ***" >> $BUILDLOGS;
make clean;
make CODE_COVERAGE=1 >>$BUILDLOGS 2>&1;
checkError $? "Makefile build failed for unittests" "Compilation error" "Check the error in $BUILDLOGS"
else
echo "***************************** Building unittests ***";
make clean;
make CODE_COVERAGE=1 1>>$BUILDLOGS;
checkError $? "Makefile build failed for unittests" "Compilation error" "Check the errors displayed in this window"
fi
