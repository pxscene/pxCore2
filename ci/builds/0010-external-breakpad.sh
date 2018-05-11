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

./examples/pxScene2d/external/breakpad/build.sh "$@"

