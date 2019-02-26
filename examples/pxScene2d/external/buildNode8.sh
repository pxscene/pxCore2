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

make_parallel=3

if [ "$(uname)" = "Darwin" ]; then
    make_parallel="$(sysctl -n hw.ncpu)"
elif [ "$(uname)" = "Linux" ]; then
    make_parallel="$(cat /proc/cpuinfo | grep '^processor' | wc --lines)"
fi

#--------- LIBNODE

if [ ! -e libnode-v8.11.2/libnode.dylib ] ||
   [ "$(uname)" != "Darwin" ]
then

  banner "NODE 8"

  curl -O https://nodejs.org/download/release/v8.11.2/node-v8.11.2.tar.gz
  tar xzf node-v8.11.2.tar.gz
  rm node-v8.11.2.tar.gz
  mv node-v8.11.2 libnode-v8.11.2
  cd libnode-v8.11.2
  git apply ../node-v8.11.2_mods.patch
  ./configure --shared
  make "-j${make_parallel}"
  ln -sf out/Release/obj.target/libnode.so.57 libnode.so.57
  ln -sf libnode.so.57 libnode.so
  ln -sf out/Release/libnode.57.dylib libnode.57.dylib
  ln -sf libnode.57.dylib libnode.dylib
  cd ..

fi

#--------

