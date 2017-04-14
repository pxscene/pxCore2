#!/bin/sh
BUILDLOGS=$TRAVIS_BUILD_DIR/logs/build_logs
checkError()
{
  if [ "$1" -ne 0 ]
  then
  echo "Build failed with errors !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!";
  exit 1;
  fi
}
cd $TRAVIS_BUILD_DIR/src
if [ "$TRAVIS_PULL_REQUEST" = "false" ]
then
echo "***************************** Building pxcore and rtcore ****" >> $BUILDLOGS
make -f Makefile.glut all CODE_COVERAGE=1 >>$BUILDLOGS 2>&1;
checkError $?
make -f Makefile.glut rtcore CODE_COVERAGE=1 >>$BUILDLOGS 2>&1;
checkError $?
else
echo "***************************** Building pxcore and rtcore ****"
make -f Makefile.glut all CODE_COVERAGE=1 1>>$BUILDLOGS;
checkError $?
make -f Makefile.glut rtcore CODE_COVERAGE=1 1>>$BUILDLOGS;
checkError $?
fi

cd $TRAVIS_BUILD_DIR/examples/pxScene2d/src
if [ "$TRAVIS_PULL_REQUEST" = "false" ]
then
echo "***************************** Building libpxscene ****" >> $BUILDLOGS;
make clean;
make libs-glut CODE_COVERAGE=1 >>$BUILDLOGS 2>&1;
checkError $?
else
echo "***************************** Building libpxscene ****";
make clean;
make libs-glut CODE_COVERAGE=1 1>>$BUILDLOGS;
checkError $?
fi

if [ "$TRAVIS_PULL_REQUEST" = "false" ]
then
echo "***************************** Building pxscene app ***" >> $BUILDLOGS
make -j CODE_COVERAGE=1 >>$BUILDLOGS 2>&1
checkError $?
else
echo "***************************** Building pxscene app ***"
make -j CODE_COVERAGE=1 1>>$BUILDLOGS
checkError $?
fi

cd $TRAVIS_BUILD_DIR/tests/pxScene2d;
if [ "$TRAVIS_PULL_REQUEST" = "false" ]
then
echo "***************************** Building unittests ***" >> $BUILDLOGS;
make clean;
make CODE_COVERAGE=1 >>$BUILDLOGS 2>&1;
checkError $?
else
echo "***************************** Building unittests ***";
make clean;
make CODE_COVERAGE=1 1>>$BUILDLOGS;
checkError $?
fi
