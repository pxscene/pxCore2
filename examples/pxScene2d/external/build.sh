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

make_parallel=3

if [ "$(uname)" = "Darwin" ]; then
    make_parallel="$(sysctl -n hw.ncpu)"
elif [ "$(uname)" = "Linux" ]; then
    make_parallel="$(cat /proc/cpuinfo | grep '^processor' | wc --lines)"
fi

#--------- OPENSSL

#avoid caching for openssl
export CCACHE_DISABLE=true
cd ${OPENSSL_DIR}
if [ "$(uname)" != "Darwin" ]
then
./config -shared  --prefix=`pwd`
else
./Configure darwin64-x86_64-cc -shared --prefix=`pwd`
fi
make clean
make "-j${make_parallel}"
make install -i
rm -rf libcrypto.a
rm -rf libssl.a
rm -rf lib/libcrypto.a
rm -rf lib/libssl.a
cd ..
unset CCACHE_DISABLE
export LD_LIBRARY_PATH="${OPENSSL_DIR}/:$LD_LIBRARY_PATH"
export DYLD_LIBRARY_PATH="${OPENSSL_DIR}/:$DYLD_LIBRARY_PATH"

#--------- CURL

if [ ! -e ./curl/lib/.libs/libcurl.4.dylib ] ||
   [ "$(uname)" != "Darwin" ]
then

  banner "CURL"

  cd curl

  CPPFLAGS="-I${OPENSSL_DIR} -I${OPENSSL_DIR}/include" LDFLAGS="-L${OPENSSL_DIR}/lib -Wl,-rpath,${OPENSSL_DIR}/lib " LIBS="-ldl -lpthread"  ./configure --with-ssl="${OPENSSL_DIR}"
  if [ "$(uname)" = "Darwin" ]; then
    #Removing api definition for Yosemite compatibility.
    sed -i '' '/#define HAVE_CLOCK_GETTIME_MONOTONIC 1/d' lib/curl_config.h
  fi


  make all "-j${make_parallel}"
  cd ..

fi

#--------- PNG

if [ ! -e ./libpng-1.6.28/.libs/libpng16.16.dylib ] ||
   [ "$(uname)" != "Darwin" ]
then

  banner "PNG"

  cd png
  ./configure
  make all "-j${make_parallel}"
  cd ..

fi

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

#--------- FT

if [ ! -e ./ft/objs/.libs/libfreetype.6.dylib ] ||
   [ "$(uname)" != "Darwin" ]
then

  banner "FT"

  cd ft
  export LIBPNG_LIBS="-L../png/.libs -lpng16"
  ./configure --with-png=no --with-harfbuzz=no
  make all "-j${make_parallel}"
  cd ..

fi

#--------- JPG

if [ ! -e ./jpg/.libs/libjpeg.9.dylib ] ||
   [ "$(uname)" != "Darwin" ]
then

  banner "JPG"

  cd jpg
  ./configure
  make all "-j${make_parallel}"
  cd ..

fi

#--------- ZLIB

if [ ! -e ./zlib/libz.1.2.11.dylib ] ||
   [ "$(uname)" != "Darwin" ]
then

  banner "ZLIB"

  cd zlib
  ./configure
  make all "-j${make_parallel}"
  cd ..

fi

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
  ./configure
  make "-j${make_parallel}"
  cd ..

fi

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
  ./configure --shared --shared-openssl --shared-openssl-includes="${OPENSSL_DIR}/include/" --shared-openssl-libpath="${OPENSSL_DIR}/lib"
  make "-j${make_parallel}"

  if [ "$(uname)" != "Darwin" ]
  then
    ln -sf out/Release/obj.target/libnode.so.* ./
    ln -sf libnode.so.* libnode.so
  else
    ln -sf out/Release/libnode.*.dylib ./
    ln -sf libnode.*.dylib libnode.dylib
  fi

  cd ..
  rm node
  ln -sf "libnode-v${NODE_VER}" node
fi

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

  make
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

#-------- NANOSVG

  banner "NANOSVG"

./nanosvg/build.sh

#-------- DUKTAPE

if [ ! -e dukluv/build/libduktape.a ]
then
  banner "DUCKTAPE"

  ./dukluv/build.sh
fi

#-------- spark-webgl
export NODE_PATH=$NODE_PATH:`pwd`/../src/node_modules
export PATH=`pwd`/node/deps/npm/bin/node-gyp-bin/:`pwd`/node/out/Release:$PATH
cd spark-webgl
node-gyp rebuild
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
