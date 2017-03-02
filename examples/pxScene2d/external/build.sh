#!/bin/bash
set -e
# Any subsequent(*) commands which fail will cause the shell script to exit immediately

#--------- CURL 
cd curl

if [ "$(uname)" == "Darwin" ]; then
./configure --with-darwinssl
else
./configure
fi

make all -j3
cd ..

#--------- PNG 
cd png
./configure
make all -j3
cd ..

#--------- FT 

cd ft
export LIBPNG_LIBS="-L../png/.libs -lpng16"
./configure --with-png=no
make all -j3
cd ..

#--------- JPG 
cd jpg
./configure
make all -j3
cd ..

#--------- ZLIB 
cd zlib
./configure
make all -j3
cd ..

#--------- LIBJPEG TURBO
if [ "$(uname)" != "Darwin" ]; then
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
cd libnode-v6.9.0
./configure --shared
make -j3
ln -sf libnode.so.48 out/Release/obj.target/libnode.so
ln -sf libnode.48.dylib out/Release/libnode.dylib
cd ..
cd libnode
./configure 
make -j3
cd ..
