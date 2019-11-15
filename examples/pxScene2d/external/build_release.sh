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

ln -s ${OPENSSL_DIR} openssl

if [ "$(uname)" != "Darwin" ]; then
  cp -R $RELEASE_EXTERNALS_PATH/extlibs/include/breakpad/* breakpad/.
fi

cp $RELEASE_EXTERNALS_PATH/extlibs/include/nanosvg/nanosvgrast.h nanosvg/src/.

cd giflib-5.1.9
[ -d .libs ] || mkdir -p .libs
if [ "$(uname)" != "Darwin" ]
then
  cp $RELEASE_EXTERNALS_PATH/extlibs/lib/libgif.so.7 .
  cp $RELEASE_EXTERNALS_PATH/extlibs/lib/libgif.so.7 .libs/.
  cp $RELEASE_EXTERNALS_PATH/extlibs/lib/libutil.so.7 .
  cp $RELEASE_EXTERNALS_PATH/extlibs/lib/libutil.so.7 .libs/.
  ln -s libgif.so.7 libgif.so
  ln -s libutil.so.7 libutil.so
  cd .libs
  ln -s libgif.so.7 libgif.so
  ln -s libutil.so.7 libutil.so
  cd ..
else
  cp $RELEASE_EXTERNALS_PATH/extlibs/lib/libgif.7.dylib .
  cp $RELEASE_EXTERNALS_PATH/extlibs/lib/libgif.7.dylib .libs/.
  cp $RELEASE_EXTERNALS_PATH/extlibs/lib/libutil.7.dylib .
  cp $RELEASE_EXTERNALS_PATH/extlibs/lib/libutil.7.dylib .libs/.
  ln -s libgif.7.dylib libgif.dylib 
  ln -s libutil.7.dylib libutil.dylib 
  cd .libs
  ln -s libgif.7.dylib libgif.dylib 
  ln -s libutil.7.dylib libutil.dylib 
  cd ..
fi
cd ..

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
cp $RELEASE_EXTERNALS_PATH/extlibs/include/node/node.h src/.
cp $RELEASE_EXTERNALS_PATH/extlibs/include/node/node_contextify_mods.h src/.
cp $RELEASE_EXTERNALS_PATH/extlibs/include/node/node_internals.h src/.
cp $RELEASE_EXTERNALS_PATH/extlibs/include/node/module_wrap.h src/.
cp $RELEASE_EXTERNALS_PATH/extlibs/include/node/env-inl.h src/.
cp $RELEASE_EXTERNALS_PATH/extlibs/include/node/env.h src/.
cp $RELEASE_EXTERNALS_PATH/extlibs/include/node/stream_wrap.h src/.
cp $RELEASE_EXTERNALS_PATH/extlibs/include/node/tcp_wrap.h src/.
cp $RELEASE_EXTERNALS_PATH/extlibs/include/node/node_crypto.h src/.
cd ..
rm node
ln -sf "libnode-v${NODE_VER}" node

cd spark-webgl
if [ "$(uname)" != "Darwin" ]
then
  cp $RELEASE_EXTERNALS_PATH/extlibs/node_modules/gles2.node ../../src/node_modules/.
else
  cp $RELEASE_EXTERNALS_PATH/extlibs/node_modules/gles2.node ../../src/node_modules/.
fi
cd ..


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

  make

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

cd ..
