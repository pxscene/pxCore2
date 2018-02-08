#!/bin/sh
if ( [ "$TRAVIS_EVENT_TYPE" = "push" ] || [ "$TRAVIS_EVENT_TYPE" = "pull_request" ] ) ; then 
codecov --build "$TRAVIS_OS_NAME-$TRAVIS_COMMIT-$TRAVIS_BUILD_NUMBER" -X gcov -f $TRAVIS_BUILD_DIR/tracefile ;
fi

if ( [ "$TRAVIS_EVENT_TYPE" = "push" ] || [ "$TRAVIS_EVENT_TYPE" = "pull_request" ] ) ; then 
genhtml -o $TRAVIS_BUILD_DIR/logs/codecoverage $TRAVIS_BUILD_DIR/tracefile;
fi
