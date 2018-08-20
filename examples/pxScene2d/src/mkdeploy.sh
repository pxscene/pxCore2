#!/bin/bash


APPNAME=Spark
if [ "$TRAVIS_EVENT_TYPE" == "cron" ]
then
APPNAME=SparkEdge
fi

function mkUpdate() {

INFILE=macstuff/software_update.plist
OUTFILE=deploy/mac/software_update.plist
DMGFILE=deploy/mac/${APPNAME}.dmg

SIZE=`stat -f "%z" deploy/mac/${APPNAME}.dmg`
HASH=`openssl sha1 -binary "${DMGFILE}" | openssl base64`
VERS=$1

if [ "$TRAVIS_EVENT_TYPE" == "cron" ]
then
IMGURL="http://96.116.56.119/edge/osx/artifacts/SparkEdge.dmg"
else
IMGURL="http://www.pxscene.org/dist/osx/pxscene/Spark.dmg"
fi

printf "Updating ${FILE} for ${1}\n"
m4 -D__IMG_URL__="${IMGURL}" -D__HASH__="${HASH}" -D__SIZE__="${SIZE}" -D__VERSION__="${VERS}" ${INFILE} > ${OUTFILE}

}


DEPLOY_DIR=deploy/mac/.stage
PX_SCENE_VERSION=$1
mkdir -p $DEPLOY_DIR
rm -r $DEPLOY_DIR/${APPNAME}.app
cp -a ${APPNAME}.app $DEPLOY_DIR
echo $PX_SCENE_VERSION > $DEPLOY_DIR/${APPNAME}.app/Contents/MacOS/version

#build dmg
./mkdmg.sh
mkUpdate $PX_SCENE_VERSION
