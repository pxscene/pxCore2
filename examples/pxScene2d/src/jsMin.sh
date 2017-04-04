#!/bin/bash

if [ "$#" -lt 1 ]
then
  printf "\n Usage: cCompiler <input.js> <output.js> \n\n"
  exit 1
fi

INPUT=$1
OUTPUT=$2


if [ ! -e "$INPUT" ]
then
  printf "\nCLOSURE COMPILER - Error:   INPUT file '$INPUT' not found.\n\n"
  exit 1
fi


CLOSURE_COMPILER=closure-compiler-v20170218.jar

# Create default output filename if needed...
if [ -z "$2" ]
then
  OUTPUT="${INPUT%.*}".min.js
fi

printf "\nCLOSURE COMPILER - Processing: $INPUT  >>>  $OUTPUT  \n\n"


java -jar $CLOSURE_COMPILER --js $INPUT --js_output_file $OUTPUT

printf "\n... Done !\n\n"