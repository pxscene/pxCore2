#!/usr/bin/env bash

#Get absolute path to this script
THIS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

cd "$THIS_DIR"

echo "${BASH_SOURCE[0]}"

BASE_URL="https://etwiki.sys.comcast.net/rest/api/content/558517563"
STORAGE_URL="https://etwiki.sys.comcast.net/plugins/viewstorage/viewpagestorage.action?pageId=558517563"
#PAGE_URL="https://etwiki.sys.comcast.net/display/RDK/GPU+and+CPU+Benchmark+Tests"
FORM_PATH="/tmp/form.txt"
CONFIG_PATH="/opt/pxbenchmark.conf"
SPACE="YOUR_PERSONAL_SPACE"
DATE=(`date +"%m\/%d\/%Y"`)
USER=$(cat $CONFIG_PATH | grep ^user:$versionTag1 | cut -d ":" -f 2)
echo "USER=$USER"
PASSWORD=$(cat $CONFIG_PATH | grep ^password:$versionTag1 | cut -d ":" -f 2)
echo "PASSWORD=$PASSWORD"
FIRMWARE=$(cat $THIS_DIR/version.txt | grep ^imagename:$versionTag1 | cut -d ":" -f 2)
echo "Firmware=$FIRMWARE"

DEVICE_NAME=$(cat $THIS_DIR/version.txt | grep ^JENKINS_JOB=$versionTag1 | cut -d "=" -f 2)
echo "Device Name=$DEVICE_NAME"

if [[ $USER == '' ]]; then
echo "Insert your account name:"
read USER
fi

if [[ $PASSWORD == '' ]]; then
echo "Insert your password:"
read -s PASSWORD
fi
#Download attachment to be edited

curl -u $USER:$PASSWORD $STORAGE_URL -o $FORM_PATH
BASE_DATA=$(curl -u $USER:$PASSWORD $BASE_URL)
BASE_DATA=$(echo $BASE_DATA | tr "\," "\n")
#echo $BASE_DATA
GIFS="$OIFS"
IFS=''
declare -i VERSION
VERSION=0
VERSION=$(echo $BASE_DATA | grep ^\"number\":$versionTag1 | cut -d ":" -f 2)
VERSION=$VERSION+1
echo "VERSION="$VERSION
DATA=$(cat $FORM_PATH)

#if [[ $DATA == '' ]]; then
#    echo "Authentication failed: Please try again"
#    exit
#fi
#echo "Insert Device Name:"
#read DEVICE_NAME
#echo "Insert Firmware:"
#read FIRMWARE
#echo "Insert Version:"
#read VERSION


html_body=$(cat <<EOF
$DATA
EOF
)

# Read CSV DATA
declare -a cols
declare -i index
index=1
fieldsCount=-1
CIFS="$IFS"

CVS_DATA=$(tail $fieldsCount /tmp/pxBenchmark_outputTable.csv | while read -a line
do
echo "${line}"
done)

IFS=','
echo "CVS_DATA:${CVS_DATA[@]}"
cols=($CVS_DATA)
echo "DATE:${cols[2]} GPU:${cols[3]} CPU:${cols[4]} Notes:${cols[5]}"
IFS="$CIFS"

echo "=============BeforeUpdate:html_body=============="
#echo $html_body
echo "================================================="
html_body1=${html_body//\"/\\\"}
idx=0
declare -i icx
icx=-2
html_body2=""
while read -r line; do
    if [[ ${idx} -ne 0 ]]; then
        html_body2+="\n"
    fi

    if [[ ${icx} -eq -1 && $line == *"</tr>"* ]]; then
        #html_body2+="$line"
        icx=0
    fi

    if [[ ${icx} -eq 5 ]]; then
        echo "*** updating Notes"
        html_body2+="<td>${cols[5]}</td>\n"
        echo "<td>${cols[5]}</td>\n"
        icx=-1
    fi

    if [[ ${icx} -eq 4 ]]; then
        echo "*** updating CPU"
        html_body2+="<td>${cols[4]}</td>\n"
        echo "<td>${cols[4]}</td>\n"
        icx+=1
    fi

    if [[ ${icx} -eq 3 ]]; then
        echo "*** updating GPU"
        html_body2+="<td>${cols[3]}</td>\n"
        echo "<td>${cols[3]}</td>\n"
        icx+=1
    fi

    if [[ ${icx} -eq 2 ]]; then
        echo "*** updating date"
        icx+=1
        html_body2+="<td>${cols[2]}</td>\n"
        echo "<td>${cols[2]}</td>\n"
    fi

    if [[ ${icx} -eq 1 ]]; then
        echo "*** updating Firmware" $line
        icx+=1
        html_body2+="<td>$FIRMWARE</td>\n"
        echo "<td>$FIRMWARE</td>\n"
    fi

    if [[ $line == *$DEVICE_NAME* ]]; then
        echo "It's there, add your line instead!" $line
        icx=1
        html_body2+="$line"
    fi
#echo $line $icx
    if [[ ${icx} -eq 0 ]]; then
        html_body2+="$line"
        #echo $line
    fi

    if [[ $line == *"</tbody>"* && ${icx} -eq -1 ]]; then
        html_body2+="$line"
    fi

    if [[ $line == *"</tbody>"* && ${icx} -eq -2 ]]; then
        echo $line
        echo "Adding New Device" 
        echo "Device Type Firmware Date GPU CPU NOTES"
        echo $DEVICE_NAME $FIRMWARE ${cols[2]} ${cols[3]} ${cols[4]} ${cols[5]}
        html_body2+="<tr>\n"
        html_body2+="<td>$DEVICE_NAME</td>\n"
        html_body2+="<td>$FIRMWARE</td>\n"
        html_body2+="<td>${cols[2]}</td>\n"
        html_body2+="<td>${cols[3]}</td>\n"
        html_body2+="<td>${cols[4]}</td>\n"
        html_body2+="<td>${cols[5]}</td>\n"
        html_body2+="</tr>\n\n\n\n\n\n"
        html_body2+="$line"
        icx=0
    elif [[ ${icx} -eq -2 ]]; then
        html_body2+="$line"
        #echo "F" $line
    fi
    
    idx+=1
done <<< "$html_body1"

echo "============AfterUpdate:html_body2==============="
echo $html_body2
echo "================================================="

payload=$(cat <<EOF
{
    "id":"558517563",
    "type":"page",
    "title":"GPU+and+CPU+Benchmark",
    "body": {
        "storage": {
        "value": "$html_body2",
        "representation":"storage"
        }
    },
    "version": {
    "number": "$VERSION"
    }
}
EOF
)

echo "==========================="
#echo $payload
echo "=================================================================="

curl -u $USER:$PASSWORD -X PUT -H 'Content-Type: application/json' "https://etwiki.sys.comcast.net/rest/api/content/558517563" --data "$payload"

echo "=================================================================="


#for token in "${array[@]}"
#do
#echo -n "$token"
#done

IFS="$OIFS"
IFS="$GIFS"
