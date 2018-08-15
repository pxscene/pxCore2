#!/bin/bash


appName=Spark
if [ "$TRAVIS_EVENT_TYPE" == "cron" ]
then
appName=SparkEdge
else
appName=Spark
fi

function mkUpdate() {

INFILE=macstuff/software_update.plist
OUTFILE=deploy/mac/software_update.plist
DMGFILE=deploy/mac/${appName}.dmg

SIZE=`stat -f "%z" deploy/mac/${appName}.dmg`
HASH=`openssl sha1 -binary "${DMGFILE}" | openssl base64`
VERS=$1

printf "Updating ${FILE} for ${1}\n"
m4 -D__HASH__="${HASH}" -D__SIZE__="${SIZE}" -D__VERSION__="${VERS}" ${INFILE} > ${OUTFILE}

}


DEPLOY_DIR=deploy/mac/.stage
PX_SCENE_VERSION=$1
mkdir -p $DEPLOY_DIR
rm -r $DEPLOY_DIR/${appName}.app
cp -a ${appName}.app $DEPLOY_DIR
echo $PX_SCENE_VERSION > $DEPLOY_DIR/${appName}.app/Contents/MacOS/version

#build dmg
./mkdmg.sh
mkUpdate $PX_SCENE_VERSION
