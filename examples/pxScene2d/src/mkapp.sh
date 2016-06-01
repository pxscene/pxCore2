#!/bin/bash

externalDir=../external
bundle=pxMain.app
bundleBin=$bundle/Contents/MacOS
bundleLib=$bundleBin/lib

rm -rf $bundle

[ -d $bundleLib ] || mkdir -p $bundleLib
cp $externalDir/png/.libs/libpng16.16.dylib $bundleLib
cp $externalDir/curl/lib/.libs/libcurl.4.dylib $bundleLib
cp $externalDir/libnode/out/Release/libnode.dylib $bundleLib
cp $externalDir/ft/objs/.libs/libfreetype.6.dylib $bundleLib

cp macstuff/Info.plist $bundle/Contents
cp -a ../images $bundle/Contents
cp macstuff/FreeSans.ttf $bundleBin/FreeSans.ttf
cp browser.js $bundleBin
cp init.js $bundleBin
cp shell.js $bundleBin
cp rtNode $bundleBin
cp rtStart.sh $bundleBin
cp -a rcvrcore $bundleBin/rcvrcore
cp -a node_modules $bundleBin/node_modules
cp -a macstuff/Resources $bundleBin/Resources
