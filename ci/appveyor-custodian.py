#!/usr/bin/env python
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#
#   Author: Damian Wrobel <dwrobel@ertelnet.rybnik.pl>
#
#   Appvoyer-custodian
#
#   Takes care of number of build artifacts
#   to make sure we will not exceed available quota.
#
#
#   Environment variables:
#
#   Required
#     <API_TOKEN=AppVeyor-api-token>
#
#   Optional if runs in AppVeyor environment, otherwise required
#     APPVEYOR_ACCOUNT_NAME=your-account-name
#     APPVEYOR_PROJECT_NAME=project-name
#
#   Optional (default is 20)
#     BUILDS_TO_KEEP=number-of-builds-to-keep
#

import os
import sys
import logging
import traceback

from appveyor_client import AppveyorClient

def get_env_or_exit(name, default_value = None, exit_code = 0):
    try:
        val = os.environ[name]
    except KeyError:
        val = default_value

    if val is None:
        logging.error("Environment variable %s is not defined.", name)
        logging.info("Please define to continue. Exiting with code: %d.", exit_code)
        sys.exit(exit_code)

    #logging.debug("Environment[\'%s\']: %s", name, val)

    return val

logging.basicConfig(format='Appveyor-custodian[%(levelname)7s]: %(message)s', level=logging.DEBUG)

# based on https://www.appveyor.com/docs/environment-variables/
api_key = get_env_or_exit('API_TOKEN')
account_name = get_env_or_exit('APPVEYOR_ACCOUNT_NAME')
repo_name = get_env_or_exit('APPVEYOR_PROJECT_NAME')

try:
    logging.info("API client initialization...")
    client = AppveyorClient(api_key)

    # Get list of projects builds
    logging.info("Fetching list of builds...")
    builds = client.projects.history(account_name, repo_name, records_per_page=99999)

    builds_num = len(builds["builds"])
    logging.info("Builds: %d" % (builds_num))

    keep_builds_num = int(get_env_or_exit('BUILDS_TO_KEEP', default_value = 20))

    to_delete_num = 0

    if builds_num - keep_builds_num > 0:
        to_delete_num = builds_num - keep_builds_num

    logging.info("Builds to delete: %d" % (to_delete_num))

    for i in range(to_delete_num):
        b = builds["builds"][builds_num - (i + 1)]
        buildId = b["buildId"]
        logging.info("Deleting build: version: %s, id: %d" % (b["version"], buildId))
        client.builds.delete(account_name, buildId)
except:
    logging.error("Unexpected error:" + str(sys.exc_info()[0]))
    traceback.print_exc(file=sys.stdout)

logging.info("Done")

sys.exit(0)
