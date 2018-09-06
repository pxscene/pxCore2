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

SCAN_BUILD=(
  'scan-build'
  '-enable-checker alpha.core.BoolAssignment'

  '-enable-checker alpha.core.CastSize'
  '-enable-checker alpha.core.CastToStruct'
  '-enable-checker alpha.core.Conversion'
  '-enable-checker alpha.core.DynamicTypeChecker'
  '-enable-checker alpha.core.FixedAddr'
  '-enable-checker alpha.core.IdenticalExpr'
  '-enable-checker alpha.core.PointerArithm'
  '-enable-checker alpha.core.PointerSub'
  '-enable-checker alpha.core.SizeofPtr'
  '-enable-checker alpha.core.StackAddressAsyncEscape'
  '-enable-checker alpha.core.TestAfterDivZero'

  '-enable-checker alpha.cplusplus.DeleteWithNonVirtualDtor'
  '-enable-checker alpha.cplusplus.IteratorRange'
  '-enable-checker alpha.cplusplus.MisusedMovedObject'

  '-enable-checker alpha.deadcode.UnreachableCode'

  '-enable-checker alpha.security.ArrayBoundV2'
  '-enable-checker alpha.security.MallocOverflow'
  '-enable-checker alpha.security.ReturnPtrRange'

  '-enable-checker alpha.unix.PthreadLock'
  '-enable-checker alpha.unix.SimpleStream'
  '-enable-checker alpha.unix.Stream'
  '-enable-checker alpha.unix.cstring.BufferOverlap'
  '-enable-checker alpha.unix.cstring.NotNullTerminated'
  '-enable-checker alpha.unix.cstring.OutOfBounds'

  '--html-title=Spark--scan-build--results'
  '--force-analyze-debug-code'
  '-maxloop 32'
  '--status-bugs'
  "-o $SPARK_BASEDIR/build/scan-view"
)

${SCAN_BUILD[*]} cmake \
  ${RTREMOTE_GENERATOR_EXPORT} \
  ${spark_common_opts[*]} \
  ${spark_force_node8[*]} \
  ${spark_wayland_opts[*]} \
  -DCMAKE_CXX_FLAGS="${cxx_common_opts[*]} ${clang_specific_opts[*]}" \
  ..

${SCAN_BUILD[*]} make -j$(getconf _NPROCESSORS_ONLN) "$@"
