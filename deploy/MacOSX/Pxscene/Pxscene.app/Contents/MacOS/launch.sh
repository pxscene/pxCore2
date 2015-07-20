#!/bin/bash
base=/Applications/XRE2.app/Contents
export PATH=$base/Resources/examples/pxScene2d/external/node/bin:$base/Resources/examples/pxScene2d/external/node:$base/Resources/examples/pxScene2d/external//node-v0.12.7-darwin-x64/lib/node_modules/npm/bin/node-gyp-bin:$PATH
cd $base/Resources/examples/pxScene2d/src/jsbindings
./load.sh browser.js
