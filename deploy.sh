#!/bin/sh
export DEPLOY_DESTINATION=${DEPLOY_DESTINATION:-/var/www/html/ciresults}
export DEPLOY_USER="${DEPLOY_USER:-ubuntu}"
export CURRENT_DATE_TIME=$(date +%d%b%Y%H%M%S)
REMOTE_HOST="$1"
#since we saved $1 to REMOTE_HOST delete it from args via shift
export REMOTE_TEMPDIR=$(ssh -o StrictHostKeyChecking=no ${DEPLOY_USER}@${REMOTE_HOST} "mktemp -d")
export REMOTE_FILE_COUNT=$(ssh -o StrictHostKeyChecking=no ${DEPLOY_USER}@${REMOTE_HOST} "ls -lrt $DEPLOY_DESTINATION|wc -l")
export REMOTE_FILE_OLD=$(ssh -o StrictHostKeyChecking=no ${DEPLOY_USER}@${REMOTE_HOST} "ls -t $DEPLOY_DESTINATION|tail -1")
echo ${REMOTE_TEMPDIR}
filename=$2
scp ${filename} ${DEPLOY_USER}@${REMOTE_HOST}:${REMOTE_TEMPDIR}
SSH="ssh -tt -o StrictHostKeyChecking=no -l ${DEPLOY_USER} ${REMOTE_HOST}"
$SSH "set -e;
cd $REMOTE_TEMPDIR
mkdir $CURRENT_DATE_TIME
tar -C $CURRENT_DATE_TIME -xvzf ${filename}
mv $CURRENT_DATE_TIME/logs/* $CURRENT_DATE_TIME/.
rm -rf $CURRENT_DATE_TIME/logs
sudo mv $CURRENT_DATE_TIME  $DEPLOY_DESTINATION/.
sudo rm -rf ${REMOTE_TEMPDIR}
cd $DEPLOY_DESTINATION
if [ $REMOTE_FILE_COUNT -g 5 ]
then
  sudo rm -rf $REMOTE_FILE_OLD
fi
"
