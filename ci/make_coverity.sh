#!/bin/sh
if [ "$TRAVIS_EVENT_TYPE" = "cron" ] ; then exit 1; fi
export COVERITY_SUPPRESS_ASSERT=1
cd $TRAVIS_BUILD_DIR
mkdir temp
cd temp
cmake ..
cmake --build . --clean-first -- -j1
