#!/bin/sh
cd $TRAVIS_BUILD_DIR/src
make -f Makefile.glut clean
cd $TRAVIS_BUILD_DIR/examples/pxScene2d/src
make clean