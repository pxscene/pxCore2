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
./configure
make all -j3
cd ..

#--------- JPG 
cd jpg
./configure
make all -j3
cd ..

#--------- GIF
cd gif

if [ ! -e ./.libs/libgif.dylib ] ||
[ "$(uname)" != "Darwin" ]
then
sudo make install
[ -d .libs ] || mkdir -p .libs
if [ -e libgif.dylib ]
then
cp libgif.dylib .libs/libgif.dylib
cp libutil.dylib .libs/libutil.dylib

elif [ -e libgif.so ]
then
cp libgif.so .libs/libgif.dylib
cp libutil.so .libs/libutil.dylib
fi
fi

cd ..
#--------- ZLIB 
cd zlib
./configure
make all -j3
cd ..


#--------- LIBNODE 
cd libnode-v6.9.0
./configure --shared
make -j3
ln -sf libnode.so.48 out/Release/obj.target/libnode.so
ln -sf libnode.48.dylib out/Release/libnode.dylib
cd ..

#-------- NANOSVG

cd nanosvg
quilt push -aq 
cd ..

