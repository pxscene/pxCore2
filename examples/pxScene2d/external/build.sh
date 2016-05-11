#!/bin/bash
set -e
# Any subsequent(*) commands which fail will cause the shell script to exit immediately

#--------- CURL 
cd curl
./configure
make all -j3
cd ..

#--------- FT 
cd ft
./configure
make all -j3
cd ..

#--------- JPG 
cd jpg
./configure
make all -j3
cd ..

#--------- PNG 
cd png
./configure
make all
cd ..

#--------- ZLIB 
cd zlib
./configure
make all
cd ..

#--------- LIBNODE 
cd libnode
./configure
make all
cd ..

