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
        tar --strip-components=1 -xzvf graphite-1.3.6.tgz
        quilt push -aq || test $? = 2
        mkdir -p build

        pushd build
            PKG_CONFIG_PATH=$EXT_INSTALL_PATH/lib/pkgconfig:$PKG_CONFIG_PATH cmake -DCMAKE_VERBOSE_MAKEFILE=ON -DCMAKE_INSTALL_PREFIX=$EXT_INSTALL_PATH -DCMAKE_INSTALL_SO_NO_EXE=0 -DCMAKE_NO_SYSTEM_FROM_IMPORTED=1 -Wno-dev ..
            make -j$(getconf _NPROCESSORS_ONLN)
            make install
        popd
    fi
popd
