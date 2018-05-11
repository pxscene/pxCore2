#!/bin/bash -e
#
# Author Damian Wrobel <dwrobel@ertelnet.rybnik.pl>
#
# Runs all executable build scripts matching the following pattern:
#   [0-9]{4}-.*\.sh
#
# Usage: [./dw.sh] ./runall.sh [VERBOSE=1]
#

SCRIPT_DIR=$(cd `dirname $0` && pwd)

pushd "${SCRIPT_DIR}"

build_scripts=$(find . -type f -executable -regextype posix-extended -regex '^\./[0-9]{4}-.*\.sh' | sort -n)

for b in ${build_scripts}; do
  ./clean.sh
  echo "runall.sh: Build start: ${b}"
  time "${b}" "$@"
  echo "runall.sh: Build end: ${b}"
done

