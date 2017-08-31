#!/bin/sh
if [ "$TRAVIS_EVENT_TYPE" = "cron" ] ; then exit 1; fi
cd $TRAVIS_BUILD_DIR/src
make -f Makefile.glut
cd $TRAVIS_BUILD_DIR/examples/pxScene2d/src
make -f Makefile
