#!/bin/bash


function mkUpdate() {

APPNAME=Spark
INFILE=macstuff/software_update.plist
OUTFILE=deploy/mac/software_update.plist
DMGFILE=deploy/mac/${APPNAME}.dmg

SIZE=`stat -f "%z" deploy/mac/${APPNAME}.dmg`
HASH=`openssl sha1 -binary "${DMGFILE}" | openssl base64`
VERS=$1

printf "Updating ${FILE} for ${1}\n"
m4 -D__HASH__="${HASH}" -D__SIZE__="${SIZE}" -D__VERSION__="${VERS}" ${INFILE} > ${OUTFILE}

}


DEPLOY_DIR=deploy/mac/.stage
PX_SCENE_VERSION=$1
mkdir -p $DEPLOY_DIR
rm -r $DEPLOY_DIR/${APPNAME}.app
if [ "$TRAVIS_EVENT_TYPE" == "cron" ]
then
cp -a SparkEdge.app $DEPLOY_DIR
echo $PX_SCENE_VERSION > $DEPLOY_DIR/SparkEdge.app/Contents/MacOS/version
else
cp -a ${APPNAME}.app $DEPLOY_DIR
echo $PX_SCENE_VERSION > $DEPLOY_DIR/${APPNAME}.app/Contents/MacOS/version
fi

#build dmg
./mkdmg.sh
mkUpdate $PX_SCENE_VERSION
