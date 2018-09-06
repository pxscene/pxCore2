#!/bin/bash
#
# Copyright 2018 Damian Wrobel <dwrobel@ertelnet.rybnik.pl>
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
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

compile_rt_remote

cmake \
  ${RTREMOTE_GENERATOR_EXPORT} \
  ${spark_common_opts[*]} \
  ${spark_force_node8[*]} \
  ${spark_wayland_opts[*]} \
  -DCMAKE_CXX_FLAGS="${cxx_common_opts[*]} ${clang_specific_opts[*]} ${clang_asan_opts[*]}" \
  ..

make_build "$@"
