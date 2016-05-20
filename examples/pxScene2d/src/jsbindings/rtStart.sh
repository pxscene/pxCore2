#!/bin/sh

pxSceneDir=../..
externalDir=../../external
bindingsV8Dir=$pxSceneDir/src/jsbindings

#echo "Start"
#echo "pxSceneDir=" $pxSceneDir
#echo "jsbindingsDir=" $jsbindingsDir
#echo "\n"

export DYLD_LIBRARY_PATH=$externalDir/png/.libs/:$externalDir/curl/lib/.libs/:$externalDir/ft/objs/.libs/

# Release libs...
#
#export LD_LIBRARY_PATH=$externalDir/png/.libs/:$externalDir/jpg/.libs/:$externalDir/curl/lib/.libs/:$externalDir/libnode/out/Release/obj.target

# Debug libs...
#
export LD_LIBRARY_PATH=$externalDir/png/.libs/:$externalDir/jpg/.libs/:$externalDir/curl/lib/.libs/:$externalDir/libnode/out/Release/obj.target

#[ -f FontdinerSwanky.ttf ] || cp $pxSceneDir/src/FontdinerSwanky.ttf .
#[ -f FreeSans.ttf ] || cp $pxSceneDir/src/FreeSans.ttf .

#export NODE_PATH=$NODE_PATH:`pwd`

export RT_LOG_LEVEL=info

#../rtNode  $1 $2


#valgrind --tool=memcheck --leak-check=yes --show-reachable=yes --num-callers=20 --track-fds=yes ../rtNode $1 $2 $3 $4 $5 $6 $7
#valgrind --leak-check=yes ../rtNode $1 $2 $3 $4 $5 $6 $7


# NOTE:  To use GDB ... use the DEBUG .so path for libnode

#gdb --args ../rtNode  $1 $2
#strace -o trace.txt ../rtNode $1 $2 $3 $4 $5 $6 $7

../rtNode $1 $2 $3 $4 $5 $6 $7

#if [ "$#" -ne 1 ]; then
#   ../rtNode "./start.js"
#else
#   ../rtNode $1
#fi

##gdb --args ../rtNode start.js url=$*
#node start.js url=$*
