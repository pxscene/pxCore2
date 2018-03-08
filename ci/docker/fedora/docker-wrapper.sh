#!/bin/bash -xe
#
# Wraps commands with docker
#
# Author: Damian Wrobel <dwrobel@ertelnet.rybnik.pl>
#
# Usage: docker-wrapper.sh <command-to-execute-within-docker>
#
# Note: It has also access to the entire $HOME directory

CWD=$PWD

DIRECTORY=$(cd `dirname $0` && pwd)

DOCKER_ID=dwrobel/pxscene:28

sudo docker build -t ${DOCKER_ID} $DIRECTORY

VDIR="$HOME"

if [ -n "${DISPLAY}" ]; then
    display_opts="-e DISPLAY=$DISPLAY"
fi

if [ -n "${WAYLAND_DISPLAY}" ]; then
    wayland_display_opts="-e WAYLAND_DISPLAY=$WAYLAND_DISPLAY"
fi

if [ -n "${XDG_RUNTIME_DIR}" ]; then
    xdg_runtime_opts="-e XDG_RUNTIME_DIR=${XDG_RUNTIME_DIR} -v ${XDG_RUNTIME_DIR}:${XDG_RUNTIME_DIR}"
fi

if [ -n "${CC}" ]; then
    cc_opts="-e CC=$CC"
fi

if [ -n "${CXX}" ]; then
    cxx_opts="-e CXX=$CXX"
fi

if [ -n "${SEMAPHORE_CACHE_DIR}" ]; then
    cache_dir="-e CACHE_DIR=$SEMAPHORE_CACHE_DIR"
fi

test -t 1 && USE_TTY="-t"

sudo docker run --privileged -i ${USE_TTY} ${cache_dir} ${cc_opts} ${cxx_opts} ${wayland_display_opts} -e USER=$USER -e UID=$UID -e GID=$(id -g $USER) -e CWD="$CWD" ${display_opts} ${xdg_runtime_opts} -v /tmp/.X11-unix:/tmp/.X11-unix -v "${VDIR}":"${VDIR}" ${DOCKER_ID} "$@"
