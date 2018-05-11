#!/bin/bash
#
# Author Damian Wrobel <dwrobel@ertelnet.rybnik.pl>
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
