#!/bin/bash

#minJS=cp        #don't minify
minJS=./jsMin.sh  #minify

externalDir=../external
EXT_INSTALL_PATH=$externalDir/extlibs

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
if [ -e $externalDir/gif/.libs/libgif.7.dylib ]
then
cp $externalDir/gif/.libs/libgif.7.dylib $bundleLib
fi

cp $externalDir/png/.libs/libpng16.16.dylib $bundleLib
cp $externalDir/curl/lib/.libs/libcurl.4.dylib $bundleLib
cp $externalDir/node/out/Release/libnode*.dylib $bundleLib
cp $externalDir/ft/objs/.libs/libfreetype.6.dylib $bundleLib
cp $externalDir/jpg/.libs/libjpeg.9.dylib $bundleLib
cp $externalDir/openssl-1.0.2o/lib/libssl*.dylib $bundleLib
cp $externalDir/openssl-1.0.2o/lib/libcrypto*.dylib $bundleLib
#Avoid copying v8 artifacts if not generated
if [ -e $externalDir/v8/out.gn ]; then
 cp $externalDir/v8/out.gn/x64.release/*.bin $bundleBin
fi
cp $externalDir/sqlite/.libs/libsqlite3.dylib $bundleLib

if [[ $# -ge 1 ]] && [[ $1 == "ENABLE-AAMP" ]]; then
 find -L $EXT_INSTALL_PATH -name *.dylib
 find -L $EXT_INSTALL_PATH -name *.dylib -exec cp -PR {} $bundleLib \;
 find -L $EXT_INSTALL_PATH -name *.so -exec cp -PR {} $bundleLib \;
 cp $EXT_INSTALL_PATH/libexec/gstreamer-1.0/gst-plugin-scanner $bundleLib
 rm $bundleLib/libpng.dylib $bundleLib/libjpeg.dylib  #to avoid circular dependency
 if [ ! -e $externalDir/sqlite/.libs/libsqlite3.dylib ]; then
   rm $bundleLib/libsqlite3.dylib #to avoid conflict with sqlite lib used by CoreFoundation
 fi
fi

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
${minJS} browser/mime.js $bundleRes/browser/mime.js
${minJS} browser/editbox.js $bundleRes/browser/editbox.js
cp initGL.js $bundleRes/initGL.js
cp webgl.js $bundleRes/webgl.js
cp gles2.js $bundleRes/gles2.js

#./jsMinFolder.sh browser $bundleRes/browser

# Copy duktape modules
cp -a duk_modules $bundleRes/duk_modules
# Copy node modules
cp -a node_modules $bundleRes/node_modules
cp -a ../external/spark-webgl $bundleRes/node_modules

# Copy v8 modules
cp -a v8_modules $bundleRes/v8_modules


# Copy OTHER to Resources...
#
cp -a macstuff/Resources $bundle/Contents/Resources
