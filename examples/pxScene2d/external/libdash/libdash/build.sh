#!/bin/bash -e

# Usage: build.sh [--clean]
# Examples:
#  $ ./build.sh                # building
#  $ ./build.sh --clean        # cleaning
#  $ ./build.sh --force-clean  # force-cleaning

CWD=$PWD

DIRECTORY=$(cd `dirname $0` && pwd)
EXT_INSTALL_PATH=$PWD/extlibs

if [ "$(uname)" = "Darwin" ]; then
    LIBEXTN=dylib
elif [ "$(uname)" = "Linux" ]; then
    LIBEXTN=so
fi

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
            PKG_CONFIG_PATH=$EXT_INSTALL_PATH/lib/pkgconfig:$PKG_CONFIG_PATH cmake ..
            make -j$(getconf _NPROCESSORS_ONLN)
            install -d $EXT_INSTALL_PATH/include/libdash
            install -d $EXT_INSTALL_PATH/include/libdash/mpd
            install -d $EXT_INSTALL_PATH/include/libdash/xml
            install -d $EXT_INSTALL_PATH/include/libdash/network
            install -d $EXT_INSTALL_PATH/include/libdash/portable
            install -d $EXT_INSTALL_PATH/include/libdash/helpers
            install -d $EXT_INSTALL_PATH/include/libdash/metrics
            install -d $EXT_INSTALL_PATH/include/libdash/manager
            install -d $EXT_INSTALL_PATH/lib
			install bin/libdash.$LIBEXTN  $EXT_INSTALL_PATH/lib/libdash.$LIBEXTN
			install ../libdash/include/*.h  $EXT_INSTALL_PATH/include/libdash
			install ../libdash/source/xml/*.h  $EXT_INSTALL_PATH/include/libdash/xml
			install ../libdash/source/mpd/*.h  $EXT_INSTALL_PATH/include/libdash/mpd
			install ../libdash/source/network/*.h  $EXT_INSTALL_PATH/include/libdash/network
			install ../libdash/source/portable/*.h  $EXT_INSTALL_PATH/include/libdash/portable
			install ../libdash/source/helpers/*.h  $EXT_INSTALL_PATH/include/libdash/helpers
			install ../libdash/source/metrics/*.h  $EXT_INSTALL_PATH/include/libdash/metrics
			install ../libdash/source/manager/*.h  $EXT_INSTALL_PATH/include/libdash/manager
        popd
    fi
popd
