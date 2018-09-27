#!/bin/bash -xe
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

# Wraps commands with docker
#
# Usage: docker-wrapper.sh <command-to-execute-within-docker>
#
# Note: It has also access to the entire $HOME directory

CWD=$PWD

DIRECTORY=$(cd `dirname $0` && pwd)

if [ $# -lt 1 ]; then
    set +x
    echo ""
    echo "Docker wrapper by Damian Wrobel <dwrobel@ertelnet.rybnik.pl>"
    echo ""
    echo "      Usage: $0 <command-to-execute-within-docker>"
    echo "    Example: $0 bash"
    echo ""
    exit 1
fi

config_file="${DW_CONFIG_PATH:-${HOME}/.config/docker-wrapper.sh/dw-config.conf}"

if [ -e "${config_file}" ]; then
    # Allows to specify additional options to docker build/run commands
    # PX_DOCKER_BUILD=("--network=host" "--pull=false")
    # PX_DOCKER_RUN=("--net=host" "-v" "/data:/data")
    source "${config_file}"
fi

if [ -z "${PX_DOCKER_IMG}" ]; then
    PX_DOCKER_IMG=dwrobel/pxscene:latest
    sudo docker build --pull=true "${PX_DOCKER_BUILD[@]}" -t ${PX_DOCKER_IMG} $DIRECTORY
fi

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

sudo docker run "${PX_DOCKER_RUN[@]}" --privileged -i ${USE_TTY} ${cache_dir} ${cc_opts} ${cxx_opts} ${wayland_display_opts} -e USER=$USER -e UID=$UID -e GID=$(id -g $USER) -e CWD="$CWD" ${display_opts} ${xdg_runtime_opts} -v /tmp/.X11-unix:/tmp/.X11-unix -v "${VDIR}":"${VDIR}" ${PX_DOCKER_IMG} "$@"
