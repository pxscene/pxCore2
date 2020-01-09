#!/usr/bin/env bash
#./sparkUpdater.sh -productid org.pxscene.pxscene -version $VERSION -url ${UPDATE_URL}
#Get absolute path to this script
THIS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

cd "$THIS_DIR"

PRODUCT_ID=$2
VERSION=$4
URL=$6
CURRENT_VERSION=0
CURRENT_PRODUCT_ID=0
CURRENT_CODEBASE=""
#echo "GIVEN" $PRODUCT_ID $VERSION $URL

SOFTWARE_UPDATE_PLIST="./software_update.plist"
DATA=$(cat $SOFTWARE_UPDATE_PLIST)
#echo $DATA
#P_ID=$(cat $SOFTWARE_UPDATE_PLIST | grep ^ProductID:$tag1 | cut -d "</key> <string>" -f 2)
#echo "P_ID=$P_ID"

idx=""
regex='<string>([A-za-z0-9.\:/]+)</string>'
while read -r line; do

  if [[ $idx == "<key>ProductID</key>" ]]; then
    CURRENT_PRODUCT_ID=$line
    if [[ $CURRENT_PRODUCT_ID =~ $regex ]]
    then
        CURRENT_PRODUCT_ID=${BASH_REMATCH[1]}
    fi
    idx=""
  fi
  if [[ $line == "<key>ProductID</key>" ]]; then
    idx=$line
  fi
  if [[ $idx == "<key>Version</key>" ]]; then
    CURRENT_VERSION=$line
    if [[ $CURRENT_VERSION =~ $regex ]]
    then
        CURRENT_VERSION=${BASH_REMATCH[1]}
    fi
    idx=""
  fi
  if [[ $line == "<key>Version</key>" ]]; then
    idx=$line
  fi
  if [[ $idx == "<key>Codebase</key>" ]]; then
    CURRENT_CODEBASE=$line
    if [[ $CURRENT_CODEBASE =~ $regex ]]
    then
        CURRENT_CODEBASE=${BASH_REMATCH[1]}
    fi
    idx=""
  fi
  if [[ $line == "<key>Codebase</key>" ]]; then
    idx=$line
  fi



done <<< "$DATA"


#echo "CURRENT" $CURRENT_PRODUCT_ID $CURRENT_VERSION $CURRENT_CODEBASE
if [[ $PRODUCT_ID == $CURRENT_PRODUCT_ID ]]; then

  if [[ $CURRENT_VERSION != $VERSION ]]; then
      echo "UPDATE"
      sudo hdiutil attach $URL
  fi

fi

#echo "${BASH_SOURCE[0]}"
