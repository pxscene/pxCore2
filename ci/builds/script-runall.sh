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

#
# Runs all executable build scripts matching the following pattern:
#   [0-9]{4}-.*\.sh
#
# Usage: [./dw.sh] ./script-runall.sh [VERBOSE=1]
#

set -e
set -x

SCRIPT_DIR=$(cd `dirname $0` && pwd)

pushd "${SCRIPT_DIR}"

build_scripts=$(find . -executable -regextype posix-extended -regex '^\./[0-9]{4}-.*\.sh' -type f -or -type l | sort -n)

for b in ${build_scripts}; do
  echo "runall.sh: Build start: ${b}"
  time "${b}" "$@"
  echo "runall.sh: Build end: ${b}"
done

