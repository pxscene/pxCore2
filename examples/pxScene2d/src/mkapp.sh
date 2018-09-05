#!/bin/bash

#minJS=cp        #don't minify
minJS=./jsMin.sh  #minify

externalDir=../external
APPNAME=Spark
if [ "$TRAVIS_EVENT_TYPE" == "cron" ]
then
APPNAME=SparkEdge
fi

bundle=${APPNAME}.app
bundleBin=$bundle/Contents/MacOS

#bundleRes=$bundle/Contents/Resources
bundleRes=$bundle/Contents/MacOS
bundleCore=$bundleRes/rcvrcore
bundleUtils=$bundleCore/utils
bundleTools=$bundleCore/tools

bundleLib=$bundleBin/lib

rm -rf $bundle

# Make the bundle folder...
#
[ -d $bundleLib ] || mkdir -p $bundleLib

# Copy LIBS to Bundle...
#
cp $externalDir/png/.libs/libpng16.16.dylib $bundleLib
cp $externalDir/curl/lib/.libs/libcurl.4.dylib $bundleLib
#cp $externalDir/libnode/out/Release/libnode.dylib $bundleLib
cp $externalDir/libnode-v6.9.0/out/Release/libnode*.dylib $bundleLib
cp $externalDir/ft/objs/.libs/libfreetype.6.dylib $bundleLib
cp $externalDir/jpg/.libs/libjpeg.9.dylib $bundleLib
cp $externalDir/v8/out.gn/x64.release/*.bin $bundleBin

# Copy OTHER to Bundle...
#
cp macstuff/Info.plist $bundle/Contents

# Make the RES folders...
#
[ -d $bundleRes ] || mkdir -p $bundleRes
[ -d $bundleCore ] || mkdir -p $bundleCore
[ -d $bundleUtils ] || mkdir -p $bundleUtils
[ -d $bundleTools ] || mkdir -p $bundleTools

# Copy RESOURCES to Bundle...
#

rm -f browser/images/status_bg_edge.svg

cp -a browser $bundleRes
cp FreeSans.ttf $bundleRes
cp sparkpermissions.conf $bundleRes

cp package.json $bundleRes
if [ "$TRAVIS_EVENT_TYPE" == "cron" ]  
then
echo "************ building edge"
cp Spark $bundleBin/SparkEdge
else
cp ${APPNAME} $bundleBin
fi

if [ "$TRAVIS_EVENT_TYPE" == "cron" ]  
then
  sed -i -e 's/\.\/Spark/\.\/SparkEdge/g' macstuff/spark.sh
  sed -i -e 's/Spark.log /SparkEdge.log /g' macstuff/spark.sh
  sed -i -e 's/Spark.app /SparkEdge.app /g' macstuff/dmgresources/engine_install
  sed -i -e 's/Spark_update.log /SparkEdge_update.log /g' macstuff/dmgresources/engine_install
fi

cp macstuff/spark.sh $bundleBin
cp macstuff/EngineRunner $bundleBin

# Minify JS into Bundle...
#
# For Node
#./jsMinFolder.sh rcvrcore $bundleRes/rcvrcore
cp -a rcvrcore/* $bundleRes/rcvrcore


# NOTE" jsMin.sh will default to a 'min' name with 1 arg.  E.g.  "jsMin.sh INPUT.js"  >> INPUT.min.js
#
${minJS} init.js $bundleRes/init.js
${minJS} shell.js $bundleRes/shell.js
${minJS} browser.js $bundleRes/browser.js
${minJS} about.js $bundleRes/about.js
${minJS} browser/editbox.js $bundleRes/browser/editbox.js
${minJS} test_binding.js $bundleRes/test_binding.js
${minJS} test_module_loading.js $bundleRes/test_module_loading.js
${minJS} test_module_binding.js $bundleRes/test_module_binding.js
${minJS} test_promises.js $bundleRes/test_promises.js
#./jsMinFolder.sh browser $bundleRes/browser


# Copy duktape modules
cp -a duk_modules $bundleRes/duk_modules
# Copy node modules
cp -a node_modules $bundleRes/node_modules
# Copy v8 modules
cp -a v8_modules $bundleRes/v8_modules


# Copy OTHER to Resources...
#
cp -a macstuff/Resources $bundle/Contents/Resources
