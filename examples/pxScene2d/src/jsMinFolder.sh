#!/bin/bash

if [ "$#" -lt 1 ]
then
  printf "\n Usage: jsMinFolder.sh <folder> \n\n"
  exit 1
fi

SRC=${1%/} # removes trailing slash, if any

if [ ! -d $SRC ]
then
  printf "\n ERROR:  '$SRC' not found ! \n\n"
  exit 1
fi

#TIME_START=`date +%s`

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

# Make the DST folder... if needed
#
DST=${2%/} # removes trailing slash, if any

if [ -z "$DST" ]
then
  DST=${SRC/$SRC/"${SRC}_min"}
fi

[ -d $DST ] || mkdir -p $DST

# Scan tree for JS files...
#
src_files=`find $SRC -name "*.js" -print`

for src_name in ${src_files[@]}; do

  # Create "_min" root folder path for destination
  #
  dst_name=${src_name/$SRC/"${DST}"}

  # Google Closure Compiler
  #
  ./jsMin.sh $src_name $dst_name

#  if [ ! -e $dst_name ] || [[ $src_name -nt $dst_name ]]
#  then
#
#    # Google Closure Compiler
#    #
#    ./jsMin.sh $src_name $dst_name
#  fi

done

#TIME_END=`date +%s`
#runtime=$((TIME_END-TIME_START))

#printf "Runtime was $runtime Seconds\n"

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

