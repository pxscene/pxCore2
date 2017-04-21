#!/bin/bash

if [  -z `which java` ]
then
  printf "\nCLOSURE COMPILER - FATAL:   Java not installed !! \n\n"
exit 1
fi


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


CLOSURE_COMPILER=../external/googleJScc/closure-compiler-v20170218.jar

# Create default output filename if needed...
if [ -z "$2" ]
then
  OUTPUT="${INPUT%.*}".min.js
fi


if [[ -e $OUTPUT ]] && [[ $OUTPUT -nt $INPUT ]]
then

#  printf "\nCLOSURE COMPILER - Skipping: $INPUT  >>>  $OUTPUT  \n\n"
  exit 0
fi

#printf "\nCLOSURE COMPILER - Process: $INPUT  >>>  $OUTPUT  \n\n"

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#LEVEL=WHITESPACE_ONLY
LEVEL=SIMPLE_OPTIMIZATIONS
#LEVEL=ADVANCED_OPTIMIZATIONS

java -jar $CLOSURE_COMPILER --warning_level QUIET --compilation_level $LEVEL --js $INPUT --js_output_file $OUTPUT

# Check input & output sizes
#
INPUT_size=$(stat -f%z $INPUT)
OUTPUT_size=$(stat -f%z $OUTPUT)

# Check if Minified is *actually* smaller ?  ... or copy source file
#
if [ $OUTPUT_size -ge $INPUT_size ]
then
  cp $INPUT $OUTPUT # restore ORIGINAL file ... which is smaller in this case.
fi


# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#printf "\n... Done !\n\n"

exit 0  #success