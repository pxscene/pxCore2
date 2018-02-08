#!/bin/sh
export DEPLOY_DESTINATION=${DEPLOY_DESTINATION:-/var/www/html/ciresults}
export DEPLOY_USER="${DEPLOY_USER:-ubuntu}"
REMOTE_HOST="$1"
if [ "$DUKTAPE_SUPPORT" = "ON" ]
then
  REMOTE_DIR="${DEPLOY_DESTINATION}/${TRAVIS_BUILD_ID}-${TRAVIS_COMMIT}-${TRAVIS_OS_NAME}-duktape"
else
  REMOTE_DIR="${DEPLOY_DESTINATION}/${TRAVIS_BUILD_ID}-${TRAVIS_COMMIT}-${TRAVIS_OS_NAME}"
fi

#since we saved $1 to REMOTE_HOST delete it from args via shift
export REMOTE_FILE_COUNT=$(ssh -o StrictHostKeyChecking=no -p 2220 ${DEPLOY_USER}@${REMOTE_HOST} "ls -lrt $DEPLOY_DESTINATION|wc -l")
export REMOTE_FILE_OLD=$(ssh -o StrictHostKeyChecking=no -p 2220 ${DEPLOY_USER}@${REMOTE_HOST} "ls -t $DEPLOY_DESTINATION|tail -1")
export REMOTE_TEMPDIR=$(ssh -o StrictHostKeyChecking=no -p 2220 ${DEPLOY_USER}@${REMOTE_HOST} "mktemp -d")
echo ${REMOTE_DIR}
filename=$2
scp -P 2220 ${filename} ${DEPLOY_USER}@${REMOTE_HOST}:${REMOTE_TEMPDIR}
SSH="ssh -tt -o StrictHostKeyChecking=no -p 2220 -l ${DEPLOY_USER} ${REMOTE_HOST}"
$SSH "set -e;
sudo mkdir $REMOTE_DIR
cd $REMOTE_TEMPDIR
sudo tar -C $REMOTE_DIR -xvzf ${filename};
sudo mv $REMOTE_DIR/logs/* $REMOTE_DIR/.
sudo rm -rf $REMOTE_DIR/logs
sudo rm -rf ${REMOTE_TEMPDIR}
cd $DEPLOY_DESTINATION;
echo $REMOTE_FILE_COUNT;
if [ $REMOTE_FILE_COUNT -gt 30 ]
then
  sudo rm -rf $REMOTE_FILE_OLD
fi
"
