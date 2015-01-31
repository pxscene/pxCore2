#!/bin/sh

export LD_LIBRARY_PATH=../../external/png/.libs/

[ -f FontdinerSwanky.ttf ] || cp ../FontdinerSwanky.ttf .
[ -f FreeSans.ttf ] || cp ../FreeSans.ttf .

echo $LD_LIBRARY_PATH

JSFILE=hello.js

if [ "$1" != '' ]; then
    JSFILE=$1
fi

echo Loading $JSFILE
nodejs $JSFILE
