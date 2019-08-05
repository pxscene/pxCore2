#!/bin/sh
if [ "$TRAVIS_EVENT_TYPE" != "cron" ]
then
export DEPLOY_DESTINATION=${DEPLOY_DESTINATION:-/var/www/html/releases}
export RELEASE=1
else
export RELEASE=0
if [ "$TRAVIS_BRANCH" = "master" ]
then
export DEPLOY_DESTINATION=${DEPLOY_DESTINATION:-/var/www/html/edge}
export LATEST_IMAGE=${LATEST_IMAGE:-/var/www/html/edge/osx}
else
export DEPLOY_DESTINATION=${DEPLOY_DESTINATION:-/var/www/html/edge_webgl}
export LATEST_IMAGE=${LATEST_IMAGE:-/var/www/html/edge_webgl/osx}
fi
fi
export DEPLOY_USER="${DEPLOY_USER:-ubuntu}"
REMOTE_HOST="$1"
if [ "$TRAVIS_EVENT_TYPE" != "cron" ]
then
REMOTE_DIR="${DEPLOY_DESTINATION}/${PX_VERSION}"
else
REMOTE_DIR="${DEPLOY_DESTINATION}/${TRAVIS_BUILD_ID}-${TRAVIS_COMMIT}-${TRAVIS_OS_NAME}"
fi
#since we saved $1 to REMOTE_HOST delete it from args via shift
export REMOTE_FILE_COUNT=$(ssh -o StrictHostKeyChecking=no -p 2220 ${DEPLOY_USER}@${REMOTE_HOST} "ls -lrt $DEPLOY_DESTINATION|grep -v '^l'|grep -v total|wc -l")
export REMOTE_FILE_OLD=$(ssh -o StrictHostKeyChecking=no -p 2220 ${DEPLOY_USER}@${REMOTE_HOST} "ls -t $DEPLOY_DESTINATION|tail -1")
export REMOTE_TEMPDIR=$(ssh -o StrictHostKeyChecking=no -p 2220 ${DEPLOY_USER}@${REMOTE_HOST} "mktemp -d")
filename=$2
scp -P 2220 ${filename} ${DEPLOY_USER}@${REMOTE_HOST}:${REMOTE_TEMPDIR}
echo "${filename} ${DEPLOY_USER}@${REMOTE_HOST}:${REMOTE_TEMPDIR}"
SSH="ssh -tt -o StrictHostKeyChecking=no -p 2220 -l ${DEPLOY_USER} ${REMOTE_HOST}"
$SSH "set -e;
sudo rm -rf $REMOTE_DIR;
sudo mkdir $REMOTE_DIR;
cd $REMOTE_TEMPDIR;
echo "sudo tar -C $REMOTE_DIR -xvzf ${filename}";
sudo tar -C $REMOTE_DIR -xvzf ${filename};
sudo mv $REMOTE_DIR/release/* $REMOTE_DIR/. ;
sudo rm -rf $REMOTE_DIR/release;
sudo rm -rf ${REMOTE_TEMPDIR};
if [ $RELEASE -eq 0 ]
then
sudo rm -rf ${LATEST_IMAGE}
sudo ln -s ${REMOTE_DIR} ${LATEST_IMAGE}
fi
echo $REMOTE_FILE_COUNT;
echo $REMOTE_FILE_OLD;
if [ $REMOTE_FILE_COUNT -eq 8 ]
then
  echo \"Removing oldest file ${REMOTE_FILE_OLD}\";
  sudo rm -rf ${REMOTE_FILE_OLD};
fi
"
