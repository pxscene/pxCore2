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

set -e
set -x

SCRIPT_DIR=$(cd `dirname $0` && pwd)

pushd "${SCRIPT_DIR}"

if [ -e /etc/init.d/xvfb ]; then
    export DISPLAY=:99.0
    /bin/bash -xe /etc/init.d/xvfb start
    sleep 5 # give xvfb some time to start
fi

time ./dw.sh ./script-runall.sh "$@" $CI_EXTRA_ARG
