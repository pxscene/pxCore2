#!/bin/bash


xLIBS=${PROJECT_DIR}/lib
xEXT=${PROJECT_DIR}/../external

#echo ${xLIBS} !!!

# Test if libraries collected already ?
if [ ! -d ${xLIBS} ]; then

  mkdir $xLIBS


  echo Copying ${xLIBS} !!!

  #cp ${xEXT}/curl/lib/.libs/libcurl.a ${xLIBS}
  #cp ${xEXT}/curl/lib/.libs/libcurl.dylib ${xLIBS}
  cp ${xEXT}/curl/lib/.libs/libcurl.4.dylib ${xLIBS}

  #install_name_tool -id @executable_path/lib/libcurl.dylib ./lib/libcurl.dylib
  #install_name_tool -id @executable_path/lib/libcurl.4.dylib ./lib/libcurl.4.dylib

  #cp ${xEXT}/ft/objs/.libs/libfreetype.a ${xLIBS}
  #cp ${xEXT}/ft/objs/.libs/libfreetype.dylib ${xLIBS}
  cp ${xEXT}/ft/objs/.libs/libfreetype.6.dylib ${xLIBS}

  #install_name_tool -id @executable_path/lib/libfreetype.dylib ./lib/libfreetype.dylib
  #install_name_tool -id @executable_path/lib/libfreetype.6.dylib ./lib/libfreetype.6.dylib

  #cp ${xEXT}/jpg/.libs/libjpeg.a ${xLIBS}
  #cp ${xEXT}/jpg/.libs/libjpeg.dylib ${xLIBS}
  cp ${xEXT}/jpg/.libs/libjpeg.9.dylib ${xLIBS}

  #install_name_tool -id @executable_path/lib/libjpeg.dylib ./lib/libjpeg.dylib
  #install_name_tool -id @executable_path/lib/libjpeg.9.dylib ./lib/libjpeg.9.dylib

  #cp ${xEXT}/libnode-v6.9.0/out/Release/libnode.dylib ${xLIBS}
  cp ${xEXT}/libnode-v6.9.0/out/Release/libnode.48.dylib ${xLIBS}  #      <<<<< MISSING ???

  #install_name_tool -id @executable_path/lib/libnode.dylib ./lib/libnode.dylib
  #install_name_tool -id @executable_path/lib/libnode.48.dylib ./lib/libnode.48.dylib

  ### OLDER PNG
  ##
  #cp ${xEXT}/libpng-1.6.12/.libs/libpng16.a ${xLIBS}
  #cp ${xEXT}/libpng-1.6.12/.libs/libpng16.dylib ${xLIBS}
  #cp ${xEXT}/libpng-1.6.12/.libs/libpng16.16.dylib ${xLIBS}

  ### NEWER PNG
  ##
  #cp ${xEXT}/libpng-1.6.28/.libs/libpng16.a ${xLIBS}
  #cp ${xEXT}/libpng-1.6.28/.libs/libpng16.dylib ${xLIBS}
  cp ${xEXT}/libpng-1.6.28/.libs/libpng16.16.dylib ${xLIBS}

  #install_name_tool -id @executable_path/lib/libpng16.dylib ./lib/libpng16.dylib
  #install_name_tool -id @executable_path/lib/libpng16.16.dylib ./lib/libpng16.16.dylib

  #cp ${xEXT}/zlib/libz.a ${xLIBS}
  #cp ${xEXT}/zlib/libz.1.dylib ${xLIBS}
  cp ${xEXT}/zlib/libz.1.2.8.dylib ${xLIBS}

  #install_name_tool -id @executable_path/lib/libz.1.dylib ./lib/libz.1.dylib
  #install_name_tool -id @executable_path/lib/libz.1.2.8.dylib ./lib/libz.1.2.8.dylib

  source makeLocalLibs.sh

fi


#
#
#
