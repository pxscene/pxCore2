#!/bin/bash

echo Making Libs local to @executable_path
echo HERE `pwd`

install_name_tool -id @executable_path/lib/libcurl.4.dylib ./lib/libcurl.4.dylib

install_name_tool -id @executable_path/lib/libfreetype.6.dylib ./lib/libfreetype.6.dylib

install_name_tool -id @executable_path/lib/libjpeg.9.dylib ./lib/libjpeg.9.dylib

install_name_tool -id @executable_path/lib/libnode.48.dylib ./lib/libnode.48.dylib

install_name_tool -id @executable_path/lib/libpng16.16.dylib ./lib/libpng16.16.dylib

install_name_tool -id @executable_path/lib/libz.1.2.8.dylib ./lib/libz.1.2.8.dylib
