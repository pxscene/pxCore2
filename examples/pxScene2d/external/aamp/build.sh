#!/bin/bash -e

# Usage: build.sh [--clean]
# Examples:
#  $ ./build.sh                # building
#  $ ./build.sh --clean        # cleaning
#  $ ./build.sh --force-clean  # force-cleaning

CWD=$PWD

DIRECTORY=$(cd `dirname $0` && pwd)
EXT_INSTALL_PATH=$PWD/extlibs

pushd $DIRECTORY
    if [[ "$#" -eq "1" && "$1" == "--clean" ]]; then
        quilt pop -afq || test $? = 2
        rm -rf build
    elif [[ "$#" -eq "1" && "$1" == "--force-clean" ]]; then
        git clean -fdx .
        git checkout .
    else
        quilt push -aq || test $? = 2
        mkdir -p build

        pushd build
            PKG_CONFIG_PATH=$EXT_INSTALL_PATH/lib/pkgconfig:$PKG_CONFIG_PATH  cmake -DCMAKE_VERBOSE_MAKEFILE=ON \
            -DCMAKE_AAMP_RENDER_IN_APP=ON -DCMAKE_INSTALL_PREFIX=$EXT_INSTALL_PATH ..
            make -j$(getconf _NPROCESSORS_ONLN)
            make install
        popd
    fi
popd
