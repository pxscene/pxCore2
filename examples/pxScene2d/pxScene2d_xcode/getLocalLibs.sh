#!/bin/bash


xLIBS=${PROJECT_DIR}/lib
xEXT=${PROJECT_DIR}/../external

echo ${xLIBS} !!!

# Test if libraries collected already ?
if [ ! -d ${xLIBS} ]; then

  mkdir $xLIBS

  echo Copying ${xLIBS} !!!

  cp ${xEXT}/curl/lib/.libs/libcurl.4.dylib ${xLIBS}

  cp ${xEXT}/ft/objs/.libs/libfreetype.6.dylib ${xLIBS}

  cp ${xEXT}/jpg/.libs/libjpeg.9.dylib ${xLIBS}

  cp ${xEXT}/node/libnode.dylib ${xLIBS}

  ### OLDER PNG
  ##
  #cp ${xEXT}/libpng-1.6.12/.libs/libpng16.dylib ${xLIBS}
  #cp ${xEXT}/libpng-1.6.12/.libs/libpng16.16.dylib ${xLIBS}

  ### NEWER PNG
  ##
  cp ${xEXT}/libpng-1.6.28/.libs/libpng16.16.dylib ${xLIBS}

  cp ${xEXT}/zlib/libz.1.2.11.dylib ${xLIBS}

  cp ${xEXT}/gif/.libs/libgif.7.dylib ${xLIBS}

  cp ${xEXT}/sqlite/.libs/libsqlite3.dylib ${xLIBS}

  source makeLocalLibs.sh

fi
