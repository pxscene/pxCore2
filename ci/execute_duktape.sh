#!/bin/sh

export DUKTAPE_SUPPORT=ON
touch ~/.sparkUseDuktape
rm -rf $TRAVIS_BUILD_DIR/logs
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

sh $TRAVIS_BUILD_DIR/ci/duktape_install.sh
validateExe "$?" "duktape_install.sh"

sh $TRAVIS_BUILD_DIR/ci/script.sh
validateExe "$?" "script.sh"


if ( [ "$TRAVIS_EVENT_TYPE" = "push" ] || [ "$TRAVIS_EVENT_TYPE" = "pull_request" ] ) && [ $DUKTAPE_SUPPORT != "ON" ] ; then 
  codecov --build "$TRAVIS_OS_NAME-$TRAVIS_COMMIT-$TRAVIS_BUILD_NUMBER" -X gcov -f $TRAVIS_BUILD_DIR/tracefile ; 
fi

if ( [ "$TRAVIS_EVENT_TYPE" = "push" ] || [ "$TRAVIS_EVENT_TYPE" = "pull_request" ] ) && [ $DUKTAPE_SUPPORT != "ON" ] ; then 
  genhtml -o $TRAVIS_BUILD_DIR/logs/codecoverage $TRAVIS_BUILD_DIR/tracefile;
fi


sh $TRAVIS_BUILD_DIR/ci/after_script.sh
validateExe "$?" "after_script.sh"
