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

# Cleans build artefacts
#
# Note: It is recommended to execute it prior
# switching to use different build configuration.
#

SCRIPT_DIR=$(cd `dirname $0` && pwd)

pushd ${SCRIPT_DIR}/../../

rm -rf build
rm -f examples/pxScene2d/src/*.{a,so}
rm -f examples/pxScene2d/src/pxscene
rm -f tests/pxScene2d/pxscene2dtests
(cd remote; make clean)
