#!/bin/sh

export DUKTAPE_SUPPORT=ON
touch ~/.sparkUseDuktape
pwd

validateExe()
{
if [ "$1" -ne 0 ]
then
printf  "\n\n************** "$2" failed *******************\n\n"
fi
}

if ( [ "$TRAVIS_EVENT_TYPE" = "cron" ] || [ "$TRAVIS_EVENT_TYPE" = "api" ] ) ; then
exit 0;
fi

sh $TRAVIS_BUILD_DIR/ci/createLogsDir.sh
validateExe "$?" "createLogsDir.sh"

sh $TRAVIS_BUILD_DIR/ci/script.sh
validateExe "$?" "script.sh"

#The check [ $DUKTAPE_SUPPORT != "ON" ] needs to be removed, once code coverage starts working fine duktape.

if ( [ "$TRAVIS_EVENT_TYPE" = "push" ] || [ "$TRAVIS_EVENT_TYPE" = "pull_request" ] ) && [ $DUKTAPE_SUPPORT != "ON" ] ; then 
  codecov --build "$TRAVIS_OS_NAME-$TRAVIS_COMMIT-$TRAVIS_BUILD_NUMBER" -X gcov -f $TRAVIS_BUILD_DIR/tracefile ; 
fi

if ( [ "$TRAVIS_EVENT_TYPE" = "push" ] || [ "$TRAVIS_EVENT_TYPE" = "pull_request" ] ) && [ $DUKTAPE_SUPPORT != "ON" ] ; then 
  genhtml -o $TRAVIS_BUILD_DIR/logs/codecoverage $TRAVIS_BUILD_DIR/tracefile;
fi

mv $TRAVIS_BUILD_DIR/logs $TRAVIS_BUILD_DIR/duktape_logs

#sh $TRAVIS_BUILD_DIR/ci/after_script.sh
#validateExe "$?" "after_script.sh"
