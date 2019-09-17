#!/bin/bash

set -e
# Any subsequent(*) commands which fail will cause the shell script to exit immediately

banner() {
  msg="# $* #"
  edge=$(echo "$msg" | sed 's/./#/g')
  echo " "
  echo "$edge"
  echo "$msg"
  echo "$edge"
  echo " "
}

#--------- Args

NODE_VER="10.15.3"
OPENSSL_DIR="`pwd`/openssl-1.0.2o"
RELEASE_EXTERNALS_PATH=../rlExternals/Spark-Externals

while (( "$#" )); do
  case "$1" in
    --node-version)
      NODE_VER=$2
      shift 2
      ;;
    --) # end argument parsing
      shift
      break
      ;;
    -*|--*=) # unsupported flags
      echo "Error: Unsupported flag $1" >&2
      exit 1
      ;;
  esac
done

EXT_INSTALL_PATH=$PWD/extlibs
ln -s $RELEASE_EXTERNALS_PATH/extlibs $EXT_INSTALL_PATH

make_parallel=3

if [ "$(uname)" = "Darwin" ]; then
    make_parallel="$(sysctl -n hw.ncpu)"
    LIBEXTN=dylib
elif [ "$(uname)" = "Linux" ]; then
    make_parallel="$(cat /proc/cpuinfo | grep '^processor' | wc --lines)"
    LIBEXTN=so
fi

#--------- OPENSSL

export DISABLE_CCACHE=true
cd ${OPENSSL_DIR}
if [ "$(uname)" != "Darwin" ]
then
./config -shared  --prefix=`pwd`
else
./Configure darwin64-x86_64-cc -shared --prefix=`pwd`
fi
make clean
make "-j1"
make install -i
rm -rf libcrypto.a
rm -rf libssl.a
rm -rf lib/libcrypto.a
rm -rf lib/libssl.a
cd ..
unset DISABLE_CCACHE
ln -s ${OPENSSL_DIR} openssl
export LD_LIBRARY_PATH="${OPENSSL_DIR}/:$LD_LIBRARY_PATH"
export DYLD_LIBRARY_PATH="${OPENSSL_DIR}/:$DYLD_LIBRARY_PATH"

#--------- PNG

if [ ! -e ./libpng-1.6.28/.libs/libpng16.16.dylib ] ||
   [ "$(uname)" != "Darwin" ]
then

  banner "PNG"

  cd png
  ./configure --prefix=$EXT_INSTALL_PATH
  make all "-j${make_parallel}"
  make install
  cd ..

fi
#---------

#--------- JPG

if [ ! -e ./jpg/.libs/libjpeg.9.dylib ] ||
   [ "$(uname)" != "Darwin" ]
then

  banner "JPG"

  cd jpg
  ./configure --prefix=$EXT_INSTALL_PATH
  make all "-j${make_parallel}"
  make install
  cd ..

fi
#---------

#--------- GIF

banner "GIF"

cd gif
if [ "$(uname)" == "Darwin" ]; then

[ -d patches ] || mkdir -p patches
[ -d patches/series ] || echo 'giflib-5.1.9.patch' >patches/series
cp ../giflib-5.1.9.patch patches/

if [[ "$#" -eq "1" && "$1" == "--clean" ]]; then
	quilt pop -afq || test $? = 2
	rm -rf .libs/*
elif [[ "$#" -eq "1" && "$1" == "--force-clean" ]]; then
	git clean -fdx .
	git checkout .
	rm -rf .libs/*
else
	quilt push -aq || test $? = 2
fi

fi

if [ ! -e ./.libs/libgif.7.dylib ] ||
[ "$(uname)" != "Darwin" ]
then
    make
[ -d .libs ] || mkdir -p .libs
if [ -e libgif.7.dylib ]
then
    cp libgif.7.dylib .libs/libgif.7.dylib
    cp libutil.7.dylib .libs/libutil.7.dylib


elif [ -e libgif.so ]
then
    cp libgif.so .libs/libgif.so
    cp libutil.so .libs/libutil.so
fi
fi

cd ..

#-------- openssl

if [ ! -e $EXT_INSTALL_PATH/lib/libcrypto.$LIBEXTN ]
then
  banner "openssl"

  cp -r ${OPENSSL_DIR}/* $EXT_INSTALL_PATH
fi

#--------

#--------- CURL

if [ ! -e $EXT_INSTALL_PATH/lib/libcurl.la ]; then

  banner "CURL"

  cd curl

  CPPFLAGS="-I${OPENSSL_DIR} -I${OPENSSL_DIR}/include" LDFLAGS="-L${OPENSSL_DIR}/lib -Wl,-rpath,${OPENSSL_DIR}/lib " LIBS="-ldl -lpthread" PKG_CONFIG_PATH=$EXT_INSTALL_PATH/lib/pkgconfig:$PKG_CONFIG_PATH ./configure --with-ssl="${OPENSSL_DIR}" --prefix=$EXT_INSTALL_PATH

  if [ "$(uname)" = "Darwin" ]; then
    #Removing api definition for Yosemite compatibility.
    sed -i '' '/#define HAVE_CLOCK_GETTIME_MONOTONIC 1/d' lib/curl_config.h
  fi

  
  make all "-j${make_parallel}"
  make install
  cd ..

fi
#---------

#--------- ZLIB

if [ ! -e ./zlib/libz.1.2.11.dylib ] ||
   [ "$(uname)" != "Darwin" ]
then

  banner "ZLIB"

  cd zlib
  ./configure --prefix=$EXT_INSTALL_PATH
  make all "-j${make_parallel}"
  make install
  cd ..

fi
#---------

#--------- LIBJPEG TURBO (Non -macOS)

if [ "$(uname)" != "Darwin" ]
then

  banner "TURBO"

  cd libjpeg-turbo
  git update-index --assume-unchanged Makefile.in
  git update-index --assume-unchanged aclocal.m4
  git update-index --assume-unchanged ar-lib
  git update-index --assume-unchanged compile
  git update-index --assume-unchanged config.guess
  git update-index --assume-unchanged config.h.in
  git update-index --assume-unchanged config.sub
  git update-index --assume-unchanged configure
  git update-index --assume-unchanged depcomp
  git update-index --assume-unchanged install-sh
  git update-index --assume-unchanged java/Makefile.in
  git update-index --assume-unchanged ltmain.sh
  git update-index --assume-unchanged md5/Makefile.in
  git update-index --assume-unchanged missing
  git update-index --assume-unchanged simd/Makefile.in

  autoreconf -f -i
  ./configure --prefix=$EXT_INSTALL_PATH
  make "-j${make_parallel}"
  make install
  cd ..

fi
#---------

#--------- LIBNODE

if [ ! -e "libnode-v${NODE_VER}/libnode.dylib" ] ||
   [ "$(uname)" != "Darwin" ]
then

  banner "NODE"
  if [ -e "node-v${NODE_VER}_mods.patch" ]
  then
    git apply "node-v${NODE_VER}_mods.patch"
    git apply "openssl_1.0.2_compatibility.patch"
  fi
  cd "libnode-v${NODE_VER}"
  if [ "$(uname)" != "Darwin" ]
  then
    ln -sf $RELEASE_EXTERNALS_PATH/extlibs/lib/libnode.so.* ./
    ln -sf libnode.so.* libnode.so
    ln -sf $RELEASE_EXTERNALS_PATH/extlibs/lib/node node
  else
    ln -sf $RELEASE_EXTERNALS_PATH/extlibs/lib/libnode.*.dylib ./
    ln -sf libnode.*.dylib libnode.dylib
    ln -sf $RELEASE_EXTERNALS_PATH/extlibs/lib/node node
  fi
  cd ..
  rm node
  ln -sf "libnode-v${NODE_VER}" node
fi
export LD_LIBRARY_PATH="`pwd`/node:$LD_LIBRARY_PATH"
export DYLD_LIBRARY_PATH="`pwd`/node:$DYLD_LIBRARY_PATH"
#---------

#--------- uWebSockets
if [ ! -e ./uWebSockets/libuWS.dylib ] ||
   [ "$(uname)" != "Darwin" ]
then

  banner "uWEBSOCKETS"

  cd uWebSockets

  if [ -e Makefile.build ]
  then
    mv Makefile.build Makefile
  fi

  CPPFLAGS="-I${OPENSSL_DIR} -I${OPENSSL_DIR}/include" LDFLAGS="-L${OPENSSL_DIR}/lib -Wl,-rpath,${OPENSSL_DIR}/lib " make
  cd ..

fi
#--------

# v8
# todo - uncomment - for now build v8 with buildV8.sh directly
#bash buildV8.sh

#-------- BREAKPAD (Non -macOS)

if [ "$(uname)" != "Darwin" ]; then
  ./breakpad/build.sh
fi
#---------

#-------- NANOSVG

  banner "NANOSVG"

./nanosvg/build.sh
#---------

#-------- DUKTAPE

if [ ! -e dukluv/build/libduktape.a ]
then
  banner "DUCKTAPE"

  ./dukluv/build.sh
fi

#-------- spark-webgl
cd spark-webgl
if [ "$(uname)" != "Darwin" ]
then
  cp ../$RELEASE_EXTERNALS_PATH/extlibs/node_modules/gles2.node ../../src/node_modules/.
else
  cp ../$RELEASE_EXTERNALS_PATH/extlibs/node_modules/gles2.node ../../src/node_modules/.
fi
cd ..

#-------- 

if [ ! -e sqlite/.libs/libsqlite3.a ]
then
  banner "SQLITE"

  cd sqlite
  autoreconf -f -i
  ./configure
  make -j3
  cd ..
fi
