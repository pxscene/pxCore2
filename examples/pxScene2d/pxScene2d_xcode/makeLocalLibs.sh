#!/bin/bash

echo Making Libs local to @executable_path
echo HERE `pwd`

install_name_tool -id @executable_path/lib/libcurl.4.dylib ./lib/libcurl.4.dylib

install_name_tool -id @executable_path/lib/libfreetype.6.dylib ./lib/libfreetype.6.dylib

install_name_tool -id @executable_path/lib/libjpeg.9.dylib ./lib/libjpeg.9.dylib

# install_name_tool -id @executable_path/lib/libnode.48.dylib ./lib/libnode.48.dylib
install_name_tool -id @executable_path/lib/libnode.64.dylib ./lib/libnode.64.dylib

install_name_tool -id @executable_path/lib/libcrypto.dylib ./lib/libcrypto.dylib
install_name_tool -id @executable_path/lib/libssl.dylib ./lib/libssl.dylib

# install_name_tool -id @executable_path/lib/libssl.1.0.0.dylib ./lib/libssl.1.0.0.dylib
# install_name_tool -id @executable_path/lib/libcrypto.1.0.0.dylib ./lib/libcrypto.1.0.0.dylib



install_name_tool -id @executable_path/lib/libpng16.16.dylib ./lib/libpng16.16.dylib

install_name_tool -id @executable_path/lib/libz.1.2.11.dylib ./lib/libz.1.2.11.dylib

install_name_tool -id @executable_path/lib/libgif.7.dylib ./lib/libgif.7.dylib
install_name_tool -id @executable_path/lib/libutil.7.dylib ./lib/libutil.7.dylib

install_name_tool -id @executable_path/lib/libsqlite3.dylib ./lib/libsqlite3.dylib

install_name_tool -id @executable_path/lib/libGLEW.2.1.dylib ./lib/libGLEW.2.1.dylib

