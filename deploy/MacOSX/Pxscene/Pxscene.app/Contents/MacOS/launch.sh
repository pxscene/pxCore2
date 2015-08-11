#!/bin/bash
BASE_DIR=/Applications/Pxscene.app/Contents
PX_VERSION=`cat ${BASE_DIR}/MacOS/version`
UPDATE_URL=http://www.pxscene.org/dist/osx/Info.plist
LOG_DIR=/var/tmp/pxscene/logs
LOG_FILE=${LOG_DIR}/launch.out
DEBUG_FILE=${LOG_DIR}/debug.out
NODE_DIR=${BASE_DIR}/Resources/examples/pxScene2d/external/node
ER=`ps -Al | grep EngineRunner | grep -v grep`

test -d "${LOG_DIR}" || mkdir "${LOG_DIR}"
d=`date`
printf "${d}: ${ER}\n" >> ${LOG_FILE} 2>&1
if [ -z "${ER}" ]; then
   printf "Running EngineRunner (Update Engine)\n"
   ${BASE_DIR}/MacOS/EngineRunner run -productid com.comcast.Pxscene -version ${PX_VERSION} -url ${UPDATE_URL} >> ${{LOG_FILE}} 2>&1 &
fi
export PATH=${NODE_DIR}/bin:${NODE_DIR}/lib/node_modules/npm/bin/node-gyp-bin:${PATH}
cd ${BASE_DIR}/Resources/examples/pxScene2d/src/jsbindings
printf "Launching load.sh brower.js\n" >> ${LOG_FILE} 2>&1 
./load.sh browser.js > ${DEBUG_FILE} 2>&1
printf "load.sh browser.js done.\n" >> ${LOG_FILE} 2>&1
