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
# Docker Wrapper (dw) script
#
# Usage: dw.sh <path-to-program-to-be-executed-within-docker-image>
#

SCRIPT_DIR=$(cd `dirname $0` && pwd)

if [ $# -lt 1 ]; then
    $SCRIPT_DIR/../docker/fedora/docker-wrapper.sh bash
else
    $SCRIPT_DIR/../docker/fedora/docker-wrapper.sh "$@"
fi
