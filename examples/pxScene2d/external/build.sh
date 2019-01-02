#!/bin/bash
set -e
# Any subsequent(*) commands which fail will cause the shell script to exit immediately


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

  cd png
  ./configure
  make all "-j${make_parallel}"
  cd ..

fi

#--------- FT

if [ ! -e ./ft/objs/.libs/libfreetype.6.dylib ] ||
   [ "$(uname)" != "Darwin" ]
then

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

  cd jpg
  ./configure
  make all "-j${make_parallel}"
  cd ..

fi

#--------- ZLIB

if [ ! -e ./zlib/libz.1.2.11.dylib ] ||
   [ "$(uname)" != "Darwin" ]
then

  cd zlib
  ./configure
  make all "-j${make_parallel}"
  cd ..

fi

#--------- LIBJPEG TURBO (Non -macOS)

if [ "$(uname)" != "Darwin" ]
then

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

if [ ! -e node/libnode.dylib ] ||
   [ "$(uname)" != "Darwin" ]
then

  cd node
  ./configure --shared
  make "-j${make_parallel}"
  ln -sf out/Release/obj.target/libnode.so.48 libnode.so.48
  ln -sf libnode.so.48 libnode.so
  ln -sf out/Release/libnode.48.dylib libnode.48.dylib
  ln -sf libnode.48.dylib libnode.dylib
  cd ..

fi

#--------- uWebSockets
if [ ! -e ./uWebSockets/libuWS.dylib ] ||
   [ "$(uname)" != "Darwin" ]
then

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

./nanosvg/build.sh

#-------- DUKTAPE

if [ ! -e dukluv/build/libduktape.a ]
then
  ./dukluv/build.sh
fi

#--------

