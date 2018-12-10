#!/bin/sh
echo "{\"main\" : \"output.js\"}" > "$1/package.json"
cd $1
ls -1|grep -v 'node_modules'|grep -v 'output.js.map'|xargs jar cvf $2
cd -
