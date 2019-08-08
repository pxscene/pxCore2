#!/bin/bash -e

EXT_INSTALL_PATH=$PWD/extlibs

#Add more targets if needed
if [ "$(uname)" = "Darwin" ]; then
    TARGET=darwin64-x86_64-cc
elif [ "$(uname)" = "Linux" ]; then
    TARGET=linux-x86_64
fi

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
        tar --strip-components=1 -xzvf openssl-1.0.2o.tar.gz
        PKG_CONFIG_PATH=$EXT_INSTALL_PATH/lib/pkgconfig:$PKG_CONFIG_PATH ./Configure $TARGET shared --prefix=$EXT_INSTALL_PATH
        make -j$(getconf _NPROCESSORS_ONLN)
        make install
    fi
popd
