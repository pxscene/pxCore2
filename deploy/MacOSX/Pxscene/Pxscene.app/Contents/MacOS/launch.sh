#!/bin/bash
base=/Applications/Pxscene.app/Contents
cver=`cat $base/MacOS/version`
log=/var/tmp/xre2log/launch.out
debug=/var/tmp/xre2log/debug.out
ER=`ps -Al | grep EngineRunner | grep -v grep`
d=`date`
printf "${d}: ${ER}\n" >> ${log} 2>&1
if [ -z "${ER}" ]; then
   printf "Running EngineRunner (Update Engine)\n"
   $base/MacOS/EngineRunner run -productid com.comcast.Pxscene -version $cver -url http://www.pxscene.org/dist/osx/Info.plist >> $log 2>&1 &
fi
export PATH=$base/Resources/examples/pxScene2d/external/node/bin:$base/Resources/examples/pxScene2d/external/node:$base/Resources/examples/pxScene2d/external//node-v0.12.7-darwin-x64/lib/node_modules/npm/bin/node-gyp-bin:$PATH
cd $base/Resources/examples/pxScene2d/src/jsbindings
printf "Launching load.sh brower.js\n" >> $log 2>&1 
./load.sh browser.js > $debug 2>&1
printf "load.sh browser.js done.\n" >> $log 2>&1
