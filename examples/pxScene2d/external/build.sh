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

#--------- Args

NODE_VER="10.15.3"
OPENSSL_DIR="`pwd`/openssl-1.0.2o"

#--------- Configuration

while (( "$#" )); do
  case "$1" in
    --node-version)
      NODE_VER=$2
      shift 2
      ;;
    --) # end argument parsing
      shift
      break
      ;;
    -*|--*=) # unsupported flags
      echo "Error: Unsupported flag $1" >&2
      exit 1
      ;;
  esac
done

#--------- CURL

make_parallel=3

EXT_INSTALL_PATH=$PWD/extlibs
mkdir -p $EXT_INSTALL_PATH


if [ "$(uname)" = "Darwin" ]; then
    make_parallel="$(sysctl -n hw.ncpu)"
    LIBEXTN=dylib
elif [ "$(uname)" = "Linux" ]; then
    make_parallel="$(cat /proc/cpuinfo | grep '^processor' | wc --lines)"
    LIBEXTN=so
fi

#--------- OPENSSL

cd ${OPENSSL_DIR}
if [ "$(uname)" != "Darwin" ]
then
./config -shared  --prefix=`pwd`
else
./Configure darwin64-x86_64-cc -shared --prefix=`pwd`
fi
make clean
make "-j${make_parallel}"
make install -i
rm -rf libcrypto.a
rm -rf libssl.a
rm -rf lib/libcrypto.a
rm -rf lib/libssl.a
cd ..
export LD_LIBRARY_PATH="${OPENSSL_DIR}/:$LD_LIBRARY_PATH"
export DYLD_LIBRARY_PATH="${OPENSSL_DIR}/:$DYLD_LIBRARY_PATH"

#--------- PNG

if [ ! -e ./libpng-1.6.28/.libs/libpng16.16.dylib ] ||
   [ "$(uname)" != "Darwin" ]
then

  banner "PNG"

  cd png
  ./configure --prefix=$EXT_INSTALL_PATH
  make all "-j${make_parallel}"
  make install
  # git update-index --assume-unchanged .      # ... help GIT out
  # git update-index --assume-unchanged src    # ... help GIT out

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

#--------- GIF

banner "GIF"

cd gif

[ -d patches ] || mkdir -p patches

if [ "$(uname)" == "Darwin" ]; then
  [ -d patches/series ] || echo 'giflib-5.1.9-mac.patch' >patches/series
  cp ../giflib-5.1.9-mac.patch patches/
else
  [ -d patches/series ] || echo 'giflib-5.1.9.patch' >patches/series
  cp ../giflib-5.1.9.patch patches/
fi


if [[ "$#" -eq "1" && "$1" == "--clean" ]]; then
	quilt pop -afq || test $? = 2
	rm -rf .libs/*
elif [[ "$#" -eq "1" && "$1" == "--force-clean" ]]; then
	git clean -fdx .
	git checkout .
	rm -rf .libs/*
else
	quilt push -aq || test $? = 2
fi

if [ ! -e ./.libs/libgif.7.dylib ] ||
[ "$(uname)" != "Darwin" ]
then

  make "-j${make_parallel}"

  [ -d .libs ] || mkdir -p .libs
  if [ -e libgif.7.dylib ]
  then
      cp libgif.7.dylib .libs/libgif.7.dylib
      cp libutil.7.dylib .libs/libutil.7.dylib

  elif [ -e libgif.so ]
  then
      cp libgif.so libgif.so.7
      cp libutil.so libutil.so.7
      cp libgif.so .libs/libgif.so
      cp libutil.so .libs/libutil.so
      cp libgif.so .libs/libgif.so.7
      cp libutil.so .libs/libutil.so.7
  fi
fi

git update-index --assume-unchanged dgif_lib.c     # ... help GIT out
git update-index --assume-unchanged Makefile       # ... help GIT out

cd ..

#--------- FT

if [ ! -e ./ft/objs/.libs/libfreetype.6.dylib ] ||
   [ "$(uname)" != "Darwin" ]
then

  banner "FT"

  cd ft

  LIBPNG_LIBS="-L../png/.libs -lpng16" PKG_CONFIG_PATH=$EXT_INSTALL_PATH/lib/pkgconfig:$PKG_CONFIG_PATH ./configure --with-png=no --with-harfbuzz=no --prefix=$EXT_INSTALL_PATH
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

  cp -r ${OPENSSL_DIR}/* $EXT_INSTALL_PATH
fi

#--------

#--------- CURL

if [ ! -e $EXT_INSTALL_PATH/lib/libcurl.la ]; then

  banner "CURL"

  cd curl

  CPPFLAGS="-I${OPENSSL_DIR} -I${OPENSSL_DIR}/include" LDFLAGS="-L${OPENSSL_DIR}/lib -Wl,-rpath,${OPENSSL_DIR}/lib " LIBS="-ldl -lpthread" PKG_CONFIG_PATH=$EXT_INSTALL_PATH/lib/pkgconfig:$PKG_CONFIG_PATH ./configure --with-ssl="${OPENSSL_DIR}" --prefix=$EXT_INSTALL_PATH

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

  git update-index --assume-unchanged Makefile              # help GIT out
  git update-index --assume-unchanged zconf.h               # help GIT out

  cd ..

fi
#---------

#--------- LIBJPEG TURBO (Non -macOS)

if [ "$(uname)" != "Darwin" ]
then

  banner "TURBO"

  cd libjpeg-turbo

  git update-index --assume-unchanged Makefile.in           # ... help GIT out
  git update-index --assume-unchanged aclocal.m4            # ... help GIT out
  git update-index --assume-unchanged ar-lib                # ... help GIT out
  git update-index --assume-unchanged compile               # ... help GIT out
  git update-index --assume-unchanged config.guess          # ... help GIT out
  git update-index --assume-unchanged config.h.in           # ... help GIT out
  git update-index --assume-unchanged config.sub            # ... help GIT out
  git update-index --assume-unchanged configure             # ... help GIT out
  git update-index --assume-unchanged depcomp               # ... help GIT out
  git update-index --assume-unchanged install-sh            # ... help GIT out
  git update-index --assume-unchanged java/Makefile.in      # ... help GIT out
  git update-index --assume-unchanged ltmain.sh             # ... help GIT out
  git update-index --assume-unchanged md5/Makefile.in       # ... help GIT out
  git update-index --assume-unchanged missing               # ... help GIT out
  git update-index --assume-unchanged simd/Makefile.in      # ... help GIT out

  autoreconf -f -i
  ./configure --prefix=$EXT_INSTALL_PATH
  make "-j${make_parallel}"
  make install
  cd ..

fi
#---------

#--------- LIBNODE

if [ ! -e "libnode-v${NODE_VER}/libnode.dylib" ] ||
   [ "$(uname)" != "Darwin" ]
then

  banner "NODE"
  if [ -e "node-v${NODE_VER}_mods.patch" ]
  then
    git apply "node-v${NODE_VER}_mods.patch"
    git apply "openssl_1.0.2_compatibility.patch"
  fi

  cd "libnode-v${NODE_VER}"
  ./configure --shared --shared-openssl --shared-openssl-includes="${OPENSSL_DIR}/include/" --shared-openssl-libpath="${OPENSSL_DIR}/lib"

  make "-j${make_parallel}"

  if [ "$(uname)" != "Darwin" ]
  then
    ln -sf out/Release/obj.target/libnode.so.* ./
    ln -sf libnode.so.* libnode.so
  else
    ln -sf out/Release/libnode.*.dylib ./
    ln -sf libnode.*.dylib libnode.dylib
  fi

  cd ..
  rm node
  ln -sf "libnode-v${NODE_VER}" node
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
    cp Makefile.build Makefile
  fi

  CPPFLAGS="-I${OPENSSL_DIR} -I${OPENSSL_DIR}/include" LDFLAGS="-L${OPENSSL_DIR}/lib -Wl,-rpath,${OPENSSL_DIR}/lib " make
  cd ..

fi
#--------

# v8
# todo - uncomment - for now build v8 with buildV8.sh directly
#bash buildV8.sh

#-------- BREAKPAD (Non -macOS)

if [ "$(uname)" != "Darwin" ]; then

  cd breakpad

  ./build.sh

  cd ..
fi
#---------

#-------- NANOSVG

banner "NANOSVG"

./nanosvg/build.sh
git update-index --assume-unchanged ./nanosvg/src/nanosvgrast.h    # ... help GIT out
#---------

#-------- DUKTAPE

if [ ! -e dukluv/build/libduktape.a ]
then
  banner "DUCKTAPE"

  cd dukluv

  ./build.sh

  git update-index --assume-unchanged CMakeLists.txt   # ... help GIT out
  git update-index --assume-unchanged uv.cmake         # ... help GIT out
  git update-index --assume-unchanged src/duv.c        # ... help GIT out
  git update-index --assume-unchanged src/main.c       # ... help GIT out

  cd ..

fi

#-------- spark-webgl
export NODE_PATH=$NODE_PATH:`pwd`/../src/node_modules
export PATH=`pwd`/node/deps/npm/bin/node-gyp-bin/:`pwd`/node/out/Release:$PATH
cd spark-webgl
node-gyp rebuild
cd ..

#-------- 

#--------  SQLITE

if [ ! -e sqlite/.libs/libsqlite3.a ]
then
  banner "SQLITE"

  cd sqlite

  if [ "$(uname)" != "Darwin" ]; then   ## If non-Mac
    autoreconf -f -i
  else
    autoreconf -f
  fi

  ./configure
  make "-j${make_parallel}"

  git update-index --assume-unchanged aclocal.m4   # ... help GIT out
  git update-index --assume-unchanged compile      # ... help GIT out
  git update-index --assume-unchanged config.guess # ... help GIT out
  git update-index --assume-unchanged config.sub   # ... help GIT out
  git update-index --assume-unchanged configure    # ... help GIT out
  git update-index --assume-unchanged depcomp      # ... help GIT out
  git update-index --assume-unchanged install-sh   # ... help GIT out
  git update-index --assume-unchanged ltmain.sh    # ... help GIT out
  git update-index --assume-unchanged Makefile.in  # ... help GIT out
  git update-index --assume-unchanged missing      # ... help GIT out

  cd ..
fi

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

#-------
exit 0    #success
#-------
