#!/bin/bash
#
# Author Damian Wrobel <dwrobel@ertelnet.rybnik.pl>
#
# Docker Wrapper (dw) script
#
# Usage: dw.sh <path-to-program-to-be-executed-within-docker-image>
#
#

SCRIPT_DIR=$(cd `dirname $0` && pwd)

$SCRIPT_DIR/../docker/fedora/docker-wrapper.sh "$@"
