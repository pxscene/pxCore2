#!/bin/bash
set -e
# Any subsequent(*) commands which fail will cause the shell script to exit immediately

banner() {
  msg="# $* #"
  edge=$(echo "$msg" | sed 's/./#/g')
  echo " "
  echo "$edge"
  echo "$msg"
  echo "$edge"
  echo " "
}

EXT_INSTALL_PATH=$PWD/extlibs
mkdir -p $EXT_INSTALL_PATH

make_parallel=3

if [ "$(uname)" = "Darwin" ]; then
    make_parallel="$(sysctl -n hw.ncpu)"
    LIBEXTN=dylib
elif [ "$(uname)" = "Linux" ]; then
    make_parallel="$(cat /proc/cpuinfo | grep '^processor' | wc --lines)"
    LIBEXTN=so
fi

#--------- PNG

if [ ! -e ./libpng-1.6.28/.libs/libpng16.16.dylib ] ||
   [ "$(uname)" != "Darwin" ]
then

  banner "PNG"

  cd png
  ./configure --prefix=$EXT_INSTALL_PATH
  make all "-j${make_parallel}"
  make install
  cd ..

fi
#---------

#--------- JPG

if [ ! -e ./jpg/.libs/libjpeg.9.dylib ] ||
   [ "$(uname)" != "Darwin" ]
then

  banner "JPG"

  cd jpg
  ./configure --prefix=$EXT_INSTALL_PATH
  make all "-j${make_parallel}"
  make install
  cd ..

fi
#---------

if [[ $# -eq 1 ]] && [[ $1 == "SPARK_ENABLE_VIDEO" ]]; then
#--------graphite2

if [ ! -e $EXT_INSTALL_PATH/lib/libgraphite2.la ]
then
  banner "graphite2"

  ./graphite2/build.sh
fi

#--------

#-------- pcre

if [ ! -e $EXT_INSTALL_PATH/lib/libpcre.la ]
then
  banner "pcre"

  ./pcre/build.sh
fi

#--------

#--------icu

if [ ! -e $EXT_INSTALL_PATH/lib/libicudata.$LIBEXTN ]
then
  banner "icu"

  ./icu/build.sh
fi

#--------

#-------- libffi

if [ ! -e $EXT_INSTALL_PATH/lib/libffi.la ]
then
  banner "libffi"

  ./libffi/build.sh
fi

#--------

#--------gettext

if [ ! -e $EXT_INSTALL_PATH/lib/libintl.la ]
then
  banner "gettext"

  ./gettext/build.sh
fi

#--------

#--------glib

if [ ! -e $EXT_INSTALL_PATH/lib/libglib-2.0.la ]
then
  banner "glib"

  ./glib/build.sh
fi

#--------
fi

#--------- FT

if [ ! -e ./ft/objs/.libs/libfreetype.6.dylib ] ||
   [ "$(uname)" != "Darwin" ]
then

  banner "FT"

  cd ft
  LIBPNG_LIBS="-L../png/.libs -lpng16" PKG_CONFIG_PATH=$EXT_INSTALL_PATH/lib/pkgconfig:$PKG_CONFIG_PATH ./configure --with-png=no --prefix=$EXT_INSTALL_PATH
  make all "-j${make_parallel}"
  make install
  cd ..

fi
#---------

if [[ $# -eq 1 ]] && [[ $1 == "SPARK_ENABLE_VIDEO" ]]; then
#--------Fontconfig

if [ ! -e $EXT_INSTALL_PATH/lib/libfontconfig.$LIBEXTN ]
then
  banner "Fontconfig"

  ./fontconfig/build.sh
fi

#--------

#--------harfbuzz

if [ ! -e $EXT_INSTALL_PATH/lib/libharfbuzz.la ]
then
  banner "harfbuzz"

  ./harfbuzz/build.sh
fi
#---------
fi

#-------- openssl

if [ ! -e $EXT_INSTALL_PATH/lib/libcrypto.$LIBEXTN ]
then
  banner "openssl"

  ./openssl/build.sh
fi

#--------

#--------- CURL

if [ ! -e $EXT_INSTALL_PATH/lib/libcurl.la ]; then

  banner "CURL"

  cd curl

  PKG_CONFIG_PATH=$EXT_INSTALL_PATH/lib/pkgconfig:$PKG_CONFIG_PATH ./configure --with-ssl --prefix=$EXT_INSTALL_PATH

  if [ "$(uname)" = "Darwin" ]; then
    #Removing api definition for Yosemite compatibility.
    sed -i '' '/#define HAVE_CLOCK_GETTIME_MONOTONIC 1/d' lib/curl_config.h
  fi

  
  make all "-j${make_parallel}"
  make install
  cd ..

fi
#---------

#--------- ZLIB

if [ ! -e ./zlib/libz.1.2.11.dylib ] ||
   [ "$(uname)" != "Darwin" ]
then

  banner "ZLIB"

  cd zlib
  ./configure --prefix=$EXT_INSTALL_PATH
  make all "-j${make_parallel}"
  make install
  cd ..

fi
#---------

#--------- LIBJPEG TURBO (Non -macOS)

if [ "$(uname)" != "Darwin" ]
then

  banner "TURBO"

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
  ./configure --prefix=$EXT_INSTALL_PATH
  make "-j${make_parallel}"
  make install
  cd ..

fi
#---------

#--------- LIBNODE

if [ ! -e node/libnode.dylib ] ||
   [ "$(uname)" != "Darwin" ]
then

  banner "NODE"

  cd node
  ./configure --shared
  make "-j${make_parallel}"
  ln -sf out/Release/obj.target/libnode.so.48 libnode.so.48
  ln -sf libnode.so.48 libnode.so
  ln -sf out/Release/libnode.48.dylib libnode.48.dylib
  ln -sf libnode.48.dylib libnode.dylib
  cd ..

fi
#---------

#--------- uWebSockets
if [ ! -e ./uWebSockets/libuWS.dylib ] ||
   [ "$(uname)" != "Darwin" ]
then

  banner "uWEBSOCKETS"

  cd uWebSockets

  if [ -e Makefile.build ]
  then
    mv Makefile.build Makefile
  fi

  make
  cd ..

fi
#--------

# v8
# todo - uncomment - for now build v8 with buildV8.sh directly
#bash buildV8.sh

#-------- BREAKPAD (Non -macOS)

if [ "$(uname)" != "Darwin" ]; then
  ./breakpad/build.sh
fi
#---------

#-------- NANOSVG

  banner "NANOSVG"

./nanosvg/build.sh
#---------

#-------- DUKTAPE

if [ ! -e dukluv/build/libduktape.a ]
then
  banner "DUCKTAPE"

  ./dukluv/build.sh
fi

#--------

if [[ $# -eq 1 ]] && [[ $1 == "SPARK_ENABLE_VIDEO" ]]; then
#-------- cJSON

if [ ! -e $EXT_INSTALL_PATH/lib/libcjson.$LIBEXTN ]
then
  banner "cJSON"

  ./cJSON/build.sh
fi

#--------

#--------orc

if [ ! -e $EXT_INSTALL_PATH/lib/liborc-0.4.la ]
then
  banner "orc"

  ./orc/build.sh
fi

#--------


#--------ossp-uuid

if [ ! -e $EXT_INSTALL_PATH/lib/libuuid.la ]
then
  banner "ossp-uuid"

  ./ossp-uuid/build.sh
fi

#--------

#--------libxml2

if [ ! -e $EXT_INSTALL_PATH/lib/libxml2.la ]
then
  banner "libxml2"

  ./libxml2/build.sh
fi

#--------

#-------- libdash

if [ ! -e $EXT_INSTALL_PATH/lib/libdash.$LIBEXTN ]
then
  banner "libdash"

  ./libdash/libdash/build.sh
fi

#--------

#-------- xz-5.2.2

if [ ! -e $EXT_INSTALL_PATH/lib/liblzma.la ]
then
  banner "xz"

  ./xz/build.sh
fi

#--------

#-------- gstreamer-1.16

if [ ! -e $EXT_INSTALL_PATH/lib/libgstreamer-1.0.la ]
then
  banner "gstreamer-1.16"

  ./gstreamer/build.sh
fi

#--------

#-------- gst-plugin-base

if [ ! -e $EXT_INSTALL_PATH/lib/libgstapp-1.0.la ]
then
  banner "gst-plugins-base"

  ./gst-plugins-base/build.sh
fi

#--------
#-------- gst-plugin-bad

if [ ! -e $EXT_INSTALL_PATH/lib/libgstbadaudio-1.0.la ]
then
  banner "gst-plugins-bad"

  ./gst-plugins-bad/build.sh
fi

#--------
#-------- gst-plugin-ugly

if [ ! -e $EXT_INSTALL_PATH/lib/gstreamer-1.0/libgstx264.la ]
then
  banner "gst-plugins-ugly"

  ./gst-plugins-ugly/build.sh
fi

#--------
#-------- gst-plugin-good

if [ ! -e $EXT_INSTALL_PATH/lib/gstreamer-1.0/libgstavi.la ]
then
  banner "gst-plugins-good"

  ./gst-plugins-good/build.sh
fi

#--------
#-------- gst-libav

if [ ! -e $EXT_INSTALL_PATH/lib/gstreamer-1.0/libgstlibav.la ]
then
  banner "gst-libav"

  ./gst-libav/build.sh
fi

#--------

#-------- aampabr

if [ ! -e $EXT_INSTALL_PATH/lib/libabr.$LIBEXTN ]
then
  banner "aampabr"

  ./aampabr/build.sh
fi

#--------

#-------- aamp

if [ ! -e $EXT_INSTALL_PATH/lib/libaamp.$LIBEXTN ]
then
  banner "aamp"

  ./aamp/build.sh
fi

#--------
fi
