#!/bin/sh

set EXTDIR=../../external
LD_LIBRARY_PATH=$EXTDIR/png/.libs/

echo $LD_LIBRARY_PATH

nodejs hello.js
