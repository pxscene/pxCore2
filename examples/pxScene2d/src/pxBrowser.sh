#!/bin/bash


rm -rf plugins
mkdir plugins
mkdir plugins/platforms
cp ../external/qt-5.12.0/prebuild/mac/plugins/platforms/libqcocoa.dylib ./plugins/platforms

export LD_LIBRARY_PATH=./Spark.app/Contents/MacOS/lib.
export DYLD_LIBRARY_PATH=./Spark.app/Contents/MacOS/lib

./pxBrowser