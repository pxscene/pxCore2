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
RELEASE_EXTERNALS_PATH=`pwd`/../rlExternals/Spark-Externals

while (( "$#" )); do
  case "$1" in
    --node-version)
      NODE_VER=$2
      shift 2
      ;;
    --release-externals-path)
      RELEASE_EXTERNALS_PATH=$2
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

EXT_INSTALL_PATH=$PWD/extlibs
ln -sf $RELEASE_EXTERNALS_PATH/extlibs $EXT_INSTALL_PATH

make_parallel=3

if [ "$(uname)" = "Darwin" ]; then
    make_parallel="$(sysctl -n hw.ncpu)"
    LIBEXTN=dylib
elif [ "$(uname)" = "Linux" ]; then
    make_parallel="$(cat /proc/cpuinfo | grep '^processor' | wc --lines)"
    LIBEXTN=so
fi

#--------- OPENSSL

ln -s ${OPENSSL_DIR} openssl

#--------- CURL

if [ ! -e $EXT_INSTALL_PATH/lib/libcurl.la ]; then

  banner "CURL"

  cd curl

  if [ "$(uname)" = "Darwin" ]; then
    #Removing api definition for Yosemite compatibility.
    sed -i '' '/#define HAVE_CLOCK_GETTIME_MONOTONIC 1/d' lib/curl_config.h
  fi
  cd ..

fi

#--------- GIF

banner "GIF"

cd gif
if [ "$(uname)" == "Darwin" ]; then

[ -d patches ] || mkdir -p patches
[ -d patches/series ] || echo 'giflib-5.1.9.patch' >patches/series
cp ../giflib-5.1.9.patch patches/

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

fi

if [ -e libgif.7.dylib ]
then
    cp libgif.7.dylib .libs/libgif.7.dylib
    cp libutil.7.dylib .libs/libutil.7.dylib


elif [ -e libgif.so ]
then
    cp libgif.so .libs/libgif.so
    cp libutil.so .libs/libutil.so
fi

cd ..

#---------
#-------- BREAKPAD (Non -macOS)

if [ "$(uname)" != "Darwin" ]; then
  ./breakpad/build.sh
fi
#---------

#-------- NANOSVG

  banner "NANOSVG"

./nanosvg/build.sh
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
  if [ "$(uname)" != "Darwin" ]
  then
    ln -sf $RELEASE_EXTERNALS_PATH/extlibs/lib/libnode.so.* ./
    ln -sf libnode.so.* libnode.so
    ln -sf $RELEASE_EXTERNALS_PATH/extlibs/lib/node node
  else
    ln -sf $RELEASE_EXTERNALS_PATH/extlibs/lib/libnode.*.dylib ./
    ln -sf libnode.*.dylib libnode.dylib
    ln -sf $RELEASE_EXTERNALS_PATH/extlibs/lib/node node
  fi
  cd ..
  rm node
  ln -sf "libnode-v${NODE_VER}" node
fi
#-------- spark-webgl
cd spark-webgl
if [ "$(uname)" != "Darwin" ]
then
  cp $RELEASE_EXTERNALS_PATH/extlibs/node_modules/gles2.node ../../src/node_modules/.
else
  cp $RELEASE_EXTERNALS_PATH/extlibs/node_modules/gles2.node ../../src/node_modules/.
fi
cd ..
