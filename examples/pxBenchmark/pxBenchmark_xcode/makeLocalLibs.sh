#!/bin/bash

echo Making Libs local to @executable_path
echo HERE `pwd`

install_name_tool -id @executable_path/lib/libcurl.4.dylib ./../../pxScene2d/pxScene2d_xcode/lib/libcurl.4.dylib

install_name_tool -id @executable_path/lib/libfreetype.6.dylib ./../../pxScene2d/pxScene2d_xcode/lib/libfreetype.6.dylib

install_name_tool -id @executable_path/lib/libjpeg.9.dylib ./../../pxScene2d/pxScene2d_xcode/lib/libjpeg.9.dylib

install_name_tool -id @executable_path/lib/libnode.48.dylib ./../../pxScene2d/pxScene2d_xcode/lib/libnode.48.dylib

install_name_tool -id @executable_path/lib/libpng16.16.dylib ./../../pxScene2d/pxScene2d_xcode/lib/libpng16.16.dylib

install_name_tool -id @executable_path/lib/libz.1.2.11.dylib ./../../pxScene2d/pxScene2d_xcode/lib/libz.1.2.11.dylib
