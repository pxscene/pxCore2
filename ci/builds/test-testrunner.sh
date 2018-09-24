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
weston_pid=""
test_file=""


cleanup() {
    if [ -z "${do_cleanup}" ]; then
        return 0
    fi

    do_cleanup=""
    if [ -n "${weston_pid}" ]; then
        kill -9 $weston_pid 2>/dev/null || true
        weston_pid=""
    fi

    if [ -f "${test_file}" ]; then
        rm -f "${test_file}"
        test_file=""
    fi
}


start_display() {
    if [ -z "$DISPLAY" ]; then
        export DISPLAY=${PX_DISPLAY:-:0}
        XV=(xvfb-run -w 5 -d -s "-screen 0 1280x720x24 -ac +extension RANDR")
    fi

    if [ -z "${WAYLAND_DISPLAY}" ]; then
        if [ -n "${PX_WAYLAND_DISPLAY}" ]; then
            export WAYLAND_DISPLAY="${PX_WAYLAND_DISPLAY}"
        fi

        local wayland_display=wayland-1

        "${XV[@]}" weston --no-config --socket="${wayland_display}" &
        weston_pid=$!
        echo weston_pid=${weston_pid} || true # as on semaphoreci it sometimes generates "write error"
        sleep 5

        if ! kill -0 ${weston_pid} 2>/dev/null; then
            exit 1
        fi

        export WAYLAND_DISPLAY=${wayland_display}
    fi
}


check_result() {
    local log_file="$1"

    # Test result format:

    # TEST RESULTS:
    # Successes: 648
    # Failures: 22

    if ! grep -e "^TEST RESULTS:" >/dev/null "${log_file}"; then
        exit 1
    fi

    if ! grep -e "^Failures: [0-9].*" >/dev/null "${log_file}"; then
        exit 1
    fi

    local failures=$(grep -e "^Failures: [0-9].*" "${log_file}" | tail -n 1 | awk '{print sprintf("%d", $2)}');

    if [ $failures -ne 0 ]; then
        exit 1
    fi

    # Search for "SUMMARY: <any>Sanitizer: " errors till "TEST RESULTS:" message.
    # This effectively ignores exit-related issues.

    if sed -n "/^TEST RESULTS:/q;p" "${log_file}" | grep -e "^SUMMARY: .*Sanitizer: "; then
        exit 1
    fi

    return 0
}


test_executor() {
    local duration=600 #[s]
    dump_env
    test_file=$(mktemp -t spark-$(basename $0)-$$-XXX.log)

    configure_debugger

    configure_sanitizers

    time timeout --signal=9 ${duration} expect -n -c "set timeout ${duration}" -c "spawn bash -x ${SPARK_BASEDIR}examples/pxScene2d/src/Spark-testrunner.sh" gdb-expect.txt 2>&1 | tee "${test_file}" || true

    check_result "${test_file}"
}


run_main() {
    dump_env

    if [ -z "${DISPLAY}" -o -z "${WAYLAND_DISPLAY}" ]; then
        start_display
    fi

    test_executor
}


trap cleanup EXIT INT TERM
do_cleanup=1

run_main
