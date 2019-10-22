#!/bin/sh
if [ "$TRAVIS_EVENT_TYPE" = "cron" ] ; then exit 1; fi
export PKG_CONFIG_PATH=$TRAVIS_BUILD_DIR/examples/pxScene2d/external/extlibs/lib/pkgconfig:$PKG_CONFIG_PATH
cd $TRAVIS_BUILD_DIR
mkdir temp
cd temp
cmake -DPREFER_SYSTEM_LIBRARIES=ON -DPREFER_PKGCONFIG=ON -DPREFER_EXTERNAL_GIF=ON ..
cmake --build . --clean-first -- -j1
