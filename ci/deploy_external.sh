#!/bin/sh
export DEPLOY_DESTINATION=${DEPLOY_DESTINATION:-/var/www/html/externals}
export DEPLOY_USER="${DEPLOY_USER:-ubuntu}"
REMOTE_HOST="$1"
#REMOTE_DIR="${DEPLOY_DESTINATION}/external"
filename=$2
export REMOTE_FILE_COUNT=$(ssh -o StrictHostKeyChecking=no -p 2220 ${DEPLOY_USER}@${REMOTE_HOST} "ls -lrt $DEPLOY_DESTINATION|grep "external" |wc -l")
export REMOTE_FILE_OLD=$(ssh -o StrictHostKeyChecking=no -p 2220 ${DEPLOY_USER}@${REMOTE_HOST} "ls -t $DEPLOY_DESTINATION|grep "external"|tail -1")
scp -o StrictHostKeyChecking=no -P 2220 ${filename} ${DEPLOY_USER}@${REMOTE_HOST}:${DEPLOY_DESTINATION}
if [ "$?" -ne 0 ]
then
  echo "********************* SCP command failed ********************"
  exit 1;	
fi

SSH="ssh -tt -o StrictHostKeyChecking=no -p 2220 -l ${DEPLOY_USER} ${REMOTE_HOST}"
$SSH "set -e;
if [ $REMOTE_FILE_COUNT -gt 3 ]
then
  sudo rm -rf "/var/www/html/externals/$REMOTE_FILE_OLD"
fi
"
