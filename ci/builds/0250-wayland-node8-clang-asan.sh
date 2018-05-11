#!/bin/bash
#
# Author Damian Wrobel <dwrobel@ertelnet.rybnik.pl>
#

# Common part
SCRIPT_DIR=$(cd `dirname $0` && pwd)
source "${SCRIPT_DIR}/common.sh"

SPARK_BASEDIR=${SCRIPT_DIR}/../../
pushd $SPARK_BASEDIR

# Specific part

export CC=clang
export CXX=clang++

mkdir -p build
pushd build

cmake \
  $(compile_rt_remote) \
  ${spark_common_opts[*]} \
  ${spark_force_node8[*]} \
  ${spark_wayland_opts[*]} \
  -DCMAKE_CXX_FLAGS="${cxx_common_opts[*]} ${clang_specific_opts[*]} ${clang_asan_opts[*]}" \
  ..

make -j$(getconf _NPROCESSORS_ONLN) "$@"

