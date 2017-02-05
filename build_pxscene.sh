#!/bin/bash
set -ev
make -f Makefile.glut
make -f ./examples/pxScene2d/src/Makefile pxscene
