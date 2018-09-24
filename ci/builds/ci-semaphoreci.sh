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

#
# https://semaphoreci.com/ integration script
#
# Semaphoreci configuration setup:
#
# "Project settings"
# (1) "Build Settings"
#    (a) Leave "Setup" - leave empty
#    (b) Add new command line: ./ci/builds/ci-semaphoreci.sh
# (2) "Platform"
#    (a) Select from "DOCKER (NATIVE)" "Docker v1807"
# (3) "Environment Variables"
# (4) "Configuration files"
# (5) "Repository"
#    (a) Setup pxCore GitHub repository
# (6) "Branches"
#    (a) "Default branch" - "master"
#    (b) "Cancellation strategy" - "Don't cancel queued builds"
#    (c) "Fast failing" - "Fast failing enabled for all branches"
#    (d) "Priority branches" - leave untouched
#    (e) "Build new branches" - "Automatically"
# (7) "Build Scheduler" - leave untouched
# (8) "Docker Registry" - leave untouched
# (9) "Notifications"
#    (a) "Email" - set your e-mail
#    (b) "Webhooks" - "receive after builds" - "Failed only"
#    (c) "Campfire" - Receive after deploys" - "Failed only"
#(10) "Collaborators" - invite your friends
#(11) "Integrations" - leave untouched
#(12) "Badge" - leave untouched
#(13) "Admin"
#    (a) "Build options" - "Command timeout" - "90 minutes"
#

set -e
set -x

SCRIPT_DIR=$(cd `dirname $0` && pwd)

pushd "${SCRIPT_DIR}"

if [ -n "${SEMAPHORE}" ]; then
    free || true

    # Let's try to release some unused resources
    for s in apache2 elasticsearch cassandra mongod sphinxsearch; do
        sudo service $s stop || true
    done

    free || true

    unset DISPLAY
fi


time ./dw.sh ./script-runall.sh "$@" $CI_EXTRA_ARG

if [ -n "${SEMAPHORE}" ]; then
    free || true
fi
