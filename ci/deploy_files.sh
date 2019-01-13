#!/bin/sh
export DEPLOY_DESTINATION=${DEPLOY_DESTINATION:-/var/www/html/ciresults}
export DEPLOY_USER="${DEPLOY_USER:-ubuntu}"
REMOTE_HOST="$1"
REMOTE_DIR="${DEPLOY_DESTINATION}/${TRAVIS_BUILD_ID}-${TRAVIS_COMMIT}-${TRAVIS_OS_NAME}"
#since we saved $1 to REMOTE_HOST delete it from args via shift
export REMOTE_TEMPDIR=$(ssh -o StrictHostKeyChecking=no -p 2220 ${DEPLOY_USER}@${REMOTE_HOST} "mktemp -d")
echo ${REMOTE_DIR}
filename=$2
scp -P 2220 ${filename} ${DEPLOY_USER}@${REMOTE_HOST}:${REMOTE_TEMPDIR}
SSH="ssh -tt -o StrictHostKeyChecking=no -p 2220 -l ${DEPLOY_USER} ${REMOTE_HOST}"
$SSH "set -e;
sudo mkdir $REMOTE_DIR
cd $REMOTE_TEMPDIR
sudo mv ${filename} $REMOTE_DIR/.
sudo rm -rf ${REMOTE_TEMPDIR}
cd $DEPLOY_DESTINATION;
"
