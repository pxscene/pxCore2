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

SCRIPT_DIR=$(cd `dirname $0` && pwd)
pushd $SCRIPT_DIR

SPARK_BIN="${SPARK_BIN:-./Spark}"
SPARK_URL="${SPARK_URL:-https://px-apps.sys.comcast.net/pxscene-samples/examples/px-reference/test-run/testRunner_v7.js?tests=}"
SPARK_TEST="${SPARK_TEST:-file:../../../tests/pxScene2d/testRunner/testsDesktop.json,file:../../../tests/pxScene2d/testRunner/tests.json,file:../../../tests/pxScene2d/testRunner/testsExit.json}"

${DBG} ${SPARK_BIN} "${SPARK_URL}${SPARK_TEST}"

popd
