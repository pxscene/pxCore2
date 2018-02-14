#!/bin/sh
<<<<<<< HEAD
<<<<<<< HEAD
cd $TRAVIS_BUILD_DIR 
rm -rf temp
=======
cd $TRAVIS_BUILD_DIR/src
make -f Makefile.glut clean
cd $TRAVIS_BUILD_DIR/examples/pxScene2d/src
make clean
>>>>>>> reinstate coverity scan scripts and travis
=======
cd $TRAVIS_BUILD_DIR 
rm -rf temp
>>>>>>> Update for cmake (#873)
