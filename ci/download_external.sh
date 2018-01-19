#!/bin/sh
export DEPLOY_EXTERNAL_PATH=${DEPLOY_EXTERNAL_PATH:-http://96.116.56.119/externals/}
REMOTE_HOST="$1"
DEST_FILE="$2"
LATEST_FILE=`curl -s $DEPLOY_EXTERNAL_PATH --list-only |sort -t\> -k8 | sed -n 's/.*href=\"\(.*\.tgz\)\".*/\1/p'  | tail -1`
if [ -z "$LATEST_FILE" ]
then
  echo "***************** No Valid external present *****************"
  exit 1;  
else  
  echo "******curl -s -o $DEST_FILE/$LATEST_FILE http://96.116.56.119/externals/$LATEST_FILE*****"
  curl -s -o "$DEST_FILE/$LATEST_FILE" "http://96.116.56.119/externals/$LATEST_FILE"
  if [ "$?" -ne 0 ]
  then
    echo "************** curl command failed with error *************"
    exit 1;
  fi
  mv "$TRAVIS_BUILD_DIR/examples/pxScene2d/external" "$TRAVIS_BUILD_DIR/examples/pxScene2d/external_orig"
  tar xfz $DEST_FILE/$LATEST_FILE -C $TRAVIS_BUILD_DIR/examples/pxScene2d/
  if [ "$?" -ne 0 ]
  then
    echo "************** tar command failed with error **************"
    mv "$TRAVIS_BUILD_DIR/examples/pxScene2d/external_orig" "$TRAVIS_BUILD_DIR/examples/pxScene2d/external"
    exit 1;
  fi
fi
