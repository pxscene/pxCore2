#!/bin/bash -e

EXT_INSTALL_PATH=$PWD/extlibs

CWD=$PWD

DIRECTORY=$(cd `dirname $0` && pwd)

pushd $DIRECTORY
    if [[ "$#" -eq "1" && "$1" == "--clean" ]]; then
        quilt pop -afq || test $? = 2
        make distclean
    elif [[ "$#" -eq "1" && "$1" == "--force-clean" ]]; then
        git clean -fdx .
        git checkout .
    else
        tar --strip-components=1 -xzvf libxml2-2.9.4.tar.gz
        ./configure --prefix=$EXT_INSTALL_PATH --with-python-install-dir=$EXT_INSTALL_PATH/lib
        make -j$(getconf _NPROCESSORS_ONLN)
        make install
    fi
popd
