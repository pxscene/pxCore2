#!/bin/bash
set -e
# Any subsequent(*) commands which fail will cause the shell script to exit immediately

#--------- CURL 
cd curl
make clean -j3
cd ..

#--------- FT 
cd ft
make clean -j3
cd ..

#--------- JPG 
cd jpg
make clean -j3
cd ..

#--------- PNG 
cd png
make clean -j3
cd ..

#--------- ZLIB 
cd zlib
make distclean -j3
cd ..

#--------- LIBNODE 
cd node
make clean -j3
rm -rf icu_* config.* out/
cd ..

#--------- LIBJPEG-TURBO
if [ "$(uname)" != "Darwin" ]
then
cd libjpeg-turbo
make clean -j3
cd ..
fi

#--------- BREAKPAD (Non -macOS)
if [ "$(uname)" != "Darwin" ] 
then
cd breakpad
make clean -j3
cd ..
fi

#--------- dukluv
rm -rf dukluv/build
