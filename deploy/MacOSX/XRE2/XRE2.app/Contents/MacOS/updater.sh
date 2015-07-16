#!/bin/bash
base=/Applications/XRE2.app/Contents/MacOS
cver=`cat $base/about.txt`
$base//EngineRunner run -productid com.comcast.XRE2 -version $cver -url http://10.21.33.155/update.plist
