#!/bin/sh
BUILDLOGS=$TRAVIS_BUILD_DIR/logs/build_logs
mkdir -p $TRAVIS_BUILD_DIR/logs   # make 'logs' if necessary

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
checkError()
{
  if [ "$1" -ne 0 ]
  then
        printf "\n\n*********************************************************************";
        printf "\n******************** SCRIPT FAIL DETAILS ****************************";	
	printf "\nCI failure reason: $2"
	printf "\nCause: $3"
	printf "\nReproduction/How to fix: $4"
        printf "\n*********************************************************************";
        printf "\n*********************************************************************\n\n";
	exit 1;
  fi
}
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

export CODE_COVERAGE=1
cd $TRAVIS_BUILD_DIR
mkdir -p temp
cd temp

if [ "$TRAVIS_PULL_REQUEST" = "false" ]
then
	echo "************************* Generating config files *************************" >> $BUILDLOGS
	cmake -DBUILD_PX_TESTS=ON -DBUILD_PXSCENE_STATIC_LIB=ON -DBUILD_DEBUG_METRICS=ON -DPXSCENE_TEST_HTTP_CACHE=ON .. >>$BUILDLOGS 2>&1;
	checkError $? "cmake config failed" "Config error" "Check the error in $BUILDLOGS"

  echo "************************* Building pxcore,rtcore,pxscene app,libpxscene, unitttests ****" >> $BUILDLOGS
  cmake --build . --clean-first -- -j$(getconf _NPROCESSORS_ONLN) >>$BUILDLOGS 2>&1;
  checkError $? "cmake build failed for pxcore or rtcore" "Compilation error" "Check the error in $BUILDLOGS"

else
  echo "************************* Generating config files ****"
  cmake -DBUILD_PX_TESTS=ON -DBUILD_PXSCENE_STATIC_LIB=ON -DBUILD_DEBUG_METRICS=ON -DPXSCENE_TEST_HTTP_CACHE=ON .. 1>>$BUILDLOGS;
  checkError $? "cmake config failed" "Config error" "Check the errors displayed in this window"

	echo "************************* Building pxcore,rtcore,pxscene app,libpxscene, unitttests ********"
	cmake --build . --clean-first -- -j$(getconf _NPROCESSORS_ONLN) 1>>$BUILDLOGS;
	checkError $? "cmake build failed for pxcore,rtcore,pxscene app,libpxscene or unitttests" "Compilation error" "Check the errors displayed in this window"

fi
cd $TRAVIS_BUILD_DIR
exit 0;

