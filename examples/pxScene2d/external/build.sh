#!/bin/bash
set -e
# Any subsequent(*) commands which fail will cause the shell script to exit immediately


#--------- CURL

if [ ! -e ./curl/lib/.libs/libcurl.4.dylib ] ||
   [ "$(uname)" != "Darwin" ]
then

  cd curl

  if [ "$(uname)" == "Darwin" ]; then
    ./configure --with-darwinssl
  else
    ./configure
  fi

  make all -j3
  cd ..

fi

#--------- PNG

if [ ! -e ./libpng-1.6.28/.libs/libpng16.16.dylib ] ||
   [ "$(uname)" != "Darwin" ]
then

  cd png
  ./configure
  make all -j3
  cd ..

fi

#--------- FT

if [ ! -e ./ft/objs/.libs/libfreetype.6.dylib ] ||
   [ "$(uname)" != "Darwin" ]
then

  cd ft
  export LIBPNG_LIBS="-L../png/.libs -lpng16"
  ./configure --with-png=no
  make all -j3
  cd ..

fi

#--------- JPG

if [ ! -e ./jpg/.libs/libjpeg.9.dylib ] ||
   [ "$(uname)" != "Darwin" ]
then

  cd jpg
  ./configure
  make all -j3
  cd ..

fi

#--------- ZLIB

if [ ! -e ./zlib/libz.1.2.8.dylib ] ||
   [ "$(uname)" != "Darwin" ]
then

  cd zlib
  ./configure
  make all -j3
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
  make -j3
  cd ..

fi

#--------- LIBNODE

if [ ! -e libnode-v6.9.0/out/Release/libnode.48.dylib ] ||
   [ "$(uname)" != "Darwin" ]
then

  cd libnode-v6.9.0
  ./configure --shared
  make -j3
  ln -sf libnode.so.48 out/Release/obj.target/libnode.so
  ln -sf libnode.48.dylib out/Release/libnode.dylib
  cd ..

fi

#-------- BREAKPAD (Non -macOS)
if [ "$(uname)" != "Darwin" ]; then

  cd breakpad
  ./configure
  make
  cd ..

fi

#--------

#--------- GLUT  (Non -macOS)

# if [ "$(uname)" != "Darwin" ]
# then
#  cd freeglut
#  cmake .
#  make freeglut -j3
#  cd ..
# fi

#--------
