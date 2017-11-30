#!/bin/sh
export DEPLOY_DESTINATION=${DEPLOY_DESTINATION:-/var/www/html/releases}
export DEPLOY_USER="${DEPLOY_USER:-ubuntu}"
REMOTE_HOST="$1"
REMOTE_DIR="${DEPLOY_DESTINATION}/${PX_VERSION}"
#since we saved $1 to REMOTE_HOST delete it from args via shift
export REMOTE_FILE_COUNT=$(ssh -o StrictHostKeyChecking=no -p 2220 ${DEPLOY_USER}@${REMOTE_HOST} "ls -lrt $DEPLOY_DESTINATION|wc -l")
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
echo $REMOTE_FILE_COUNT;
if [ $REMOTE_FILE_COUNT -gt 30 ]
then
  sudo rm -rf $REMOTE_FILE_OLD;
fi
"
