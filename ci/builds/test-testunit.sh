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


do_cleanup=""
test_file=""


cleanup() {
    if [ -z "${do_cleanup}" ]; then
        return 0
    fi

    do_cleanup=""

    if [ -f "${test_file}" ]; then
        rm -f "${test_file}"
        test_file=""
    fi
}


check_result() {
    local log_file="$1"

    # Test result format:

    #[----------] Global test environment tear-down
    #[==========] 69 tests from 45 test cases ran. (42543 ms total)
    #[  PASSED  ] 69 tests.

    if ! grep -e "Global test environment tear-down" 2>/dev/null "${log_file}"; then
        exit 1
    fi

    if ! grep -e "\[  PASSED  \]" 2>/dev/null "${log_file}"; then
        exit 1
    fi

    # Search for "SUMMARY: <any>Sanitizer: " errors.

    if grep -e "^SUMMARY: .*Sanitizer: " "${log_file}"; then
        exit 1
    fi

    return 0
}


test_executor() {
    local duration=600 #[s]
    dump_env
    test_file=$(mktemp -t spark-$(basename $0)-$$-XXX.log)

    if [ -z "${DISPLAY}" ]; then
        export DISPLAY=${PX_DISPLAY:-:0}
        export XV=(xvfb-run -w 5 -d -s "-screen 0 1280x720x24 -ac +extension RANDR")
    fi

    configure_debugger

    configure_sanitizers

    time timeout --signal=9 ${duration} "${XV[@]}" expect -n -c "set timeout ${duration}" -c "spawn bash -x ${SPARK_BASEDIR}tests/pxScene2d/pxscene2dtests.sh" gdb-expect.txt 2>&1 | tee "${test_file}" || true

    check_result "${test_file}"
}


run_main() {
    test_executor
}


trap cleanup EXIT INT TERM
do_cleanup=1

run_main
