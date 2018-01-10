#!/bin/sh
export DEPLOY_DESTINATION=${DEPLOY_DESTINATION:-/var/www/html/ciresults}
export DEPLOY_USER="${DEPLOY_USER:-ubuntu}"
REMOTE_HOST="$1"
#REMOTE_DIR="${DEPLOY_DESTINATION}/external"
filename=$2
#since we saved $1 to REMOTE_HOST delete it from args via shift
scp -P 2220 ${filename} ${DEPLOY_USER}@${REMOTE_HOST}:${DEPLOY_DESTINATION}
if [ "$?" -ne 0 ]
then
  echo "******************scp failed******************"
  exit 1;	
fi
echo "***********************trace***********"
export REMOTE_FILE_COUNT=$(ssh -o StrictHostKeyChecking=no -p 2220 ${DEPLOY_USER}@${REMOTE_HOST} "ls -lrt $DEPLOY_DESTINATION|grep "external" |wc -l")
export REMOTE_FILE_OLD=$(ssh -o StrictHostKeyChecking=no -p 2220 ${DEPLOY_USER}@${REMOTE_HOST} "ls -t $DEPLOY_DESTINATION|grep "external"|tail -1")
#export REMOTE_TEMPDIR=$(ssh -o StrictHostKeyChecking=no -p 2220 ${DEPLOY_USER}@${REMOTE_HOST} "mktemp -d")
#echo ${REMOTE_DIR}
SSH="ssh -tt -o StrictHostKeyChecking=no -p 2220 -l ${DEPLOY_USER} ${REMOTE_HOST}"
$SSH "set -e;
if [ $REMOTE_FILE_COUNT -gt 1 ]
then
  sudo rm -rf $REMOTE_FILE_OLD
fi
"
