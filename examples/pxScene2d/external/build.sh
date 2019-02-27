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

#--------- CURL

make_parallel=3

if [ "$(uname)" = "Darwin" ]; then
    make_parallel="$(sysctl -n hw.ncpu)"
elif [ "$(uname)" = "Linux" ]; then
    make_parallel="$(cat /proc/cpuinfo | grep '^processor' | wc --lines)"
fi

if [ ! -e ./curl/lib/.libs/libcurl.4.dylib ] ||
   [ "$(uname)" != "Darwin" ]
then

  banner "CURL"

  cd curl

  if [ "$(uname)" = "Darwin" ]; then
    ./configure --with-darwinssl
    #Removing api definition for Yosemite compatibility.
    sed -i '' '/#define HAVE_CLOCK_GETTIME_MONOTONIC 1/d' lib/curl_config.h
  else
      if [ $(echo "$(openssl version | cut -d' ' -f 2 | cut -d. -f1-2)>1.0" | bc) ]; then
          echo "Openssl is too new for this version of libcurl.  Opting for gnutls instead..."
          ./configure --with-gnutls
      else
          echo "Using openssl < 1.1.*"
          ./configure --with-ssl
      fi

      if [ "$(cat config.log | grep '^SSL_ENABLED' | cut -d= -f 2)" != "'1'" ]; then
          echo "Failed to configure libcurl with SSL support" && exit 1
      fi
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

#--------- FT

if [ ! -e ./ft/objs/.libs/libfreetype.6.dylib ] ||
   [ "$(uname)" != "Darwin" ]
then

  banner "FT"

  cd ft
  export LIBPNG_LIBS="-L../png/.libs -lpng16"
  ./configure --with-png=no
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

nodeDir=libnode-v6.9.0
nodeVer=48

[[ -e .build_node_8.11.2 ]] && nodeDir=libnode-v8.11.2 && nodeVer=57

if [[ ${nodeVer} == 57 ]] && [[ ! -e "${nodeDir}/libnode.dylib" ]]
then

  banner "NODE 8 DOWNLOAD"

  rm -rf node-v8.11.2 libnode-v8.11.2
  curl -O https://nodejs.org/download/release/v8.11.2/node-v8.11.2.tar.gz
  tar xzf node-v8.11.2.tar.gz
  rm node-v8.11.2.tar.gz
  mv node-v8.11.2 libnode-v8.11.2
  cd libnode-v8.11.2
  git apply ../node-v8.11.2_mods.patch
  cd ..

fi

if [[ ! -e "${nodeDir}/libnode.dylib" ]] ||
   [[ "$(uname)" != "Darwin" ]]
then

  banner "NODE"

  cd "${nodeDir}"
  ./configure --shared
  make "-j${make_parallel}"
  ln -sf "out/Release/obj.target/libnode.so.${nodeVer}" "libnode.so.${nodeVer}"
  ln -sf "libnode.so.${nodeVer}" libnode.so
  ln -sf "out/Release/libnode.${nodeVer}.dylib" "libnode.${nodeVer}.dylib"
  ln -sf "libnode.${nodeVer}.dylib" libnode.dylib
  cd ..

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

#--------

