#!/bin/bash
set -ev
make -f Makefile.glut
cd $TRAVIS_BUILD_DIR/examples/pxScene2d/src
make pxscene
cd $TRAVIS_BUILD_DIR
