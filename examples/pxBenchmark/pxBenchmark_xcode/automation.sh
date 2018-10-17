#!/bin/bash
#Get absolute path to this script
THIS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

cd "$THIS_DIR"

echo "${BASH_SOURCE[0]}"

BASE_URL="https://etwiki.sys.comcast.net/rest/api/content/556737708"
STORAGE_URL="https://etwiki.sys.comcast.net/plugins/viewstorage/viewpagestorage.action?pageId=556737708"
PAGE_URL="https://etwiki.sys.comcast.net/display/RDK/GPU+and+CPU+Benchmark+Tests"
FORM_PATH="/tmp/form.txt"
SPACE="YOUR_PERSONAL_SPACE"
VERSION=0
DATE=(`date +"%m\/%d\/%Y"`)
USER=""
PASSWORD=""
DEVICE_NAME=""
#Download attachment to be edited

#FORMAT=$(curl -u akalok815:FBnl*90q*90q $BASE_URL)


#DEVICE_LIST={"Samsung Xg2v2", "Arris XG1v1", "Pace Xi3", "Cisco G8", "Pace XG2v2", "Pace XG1v3", "Arris XG1v3", "Pace XG1v1", "Raspberry Pi", "Pace Xi5", "Pace XiD", "Cisco XiD", "Arris XG1v4", "Arris Xi6"}
#DEVICE_ID=0

echo "Insert your account name:"
read USER
echo "Insert your password:"
read -s PASSWORD
echo "Insert Device Name:"
read DEVICE_NAME
echo "Insert Version:"
read VERSION

curl -u $USER:$PASSWORD $STORAGE_URL -o $FORM_PATH

#echo "Updating a page on confluence"

#TABLE=$(cat $FORM_PATH | grep "<td>.*</td>") | awk 'gsub(/<td>|<\/td>/,x)')
VALUES=$(awk -F '>|<' '{print $3 $5 ";"}' $FORM_PATH)
#$VALUES=$(echo $VALUES | tr "\"" "\\\"")
#$VALUES=$(echo $VALUES | tr " " "\\n;")
#$VALUES=$(echo $VALUES | tr "\r" "\\n;")
#Convert string to array
OIFS="$IFS"
IFS=';' #read -ra ARRAY <<< "$VALUES"


#echo ${VALUES[115]}
declare -i Index
declare -i CurrIndex
CurrIndex=-1
declare -i EndIndex
EndIndex=-2
#$VALUES=$(echo $VALUES | tr ";" "\n")
Index=0
declare -i dataIndex
declare -a array
declare -a cols
declare -i index

for token in $VALUES
do
#echo -n $Index
#echo -n "$token"
array+=( "$token" );

if [[ $token =~ .*$DEVICE_NAME.* ]]
then
    CurrentIndex=$Index
    export CurrentIndex=$CurrentIndex
    EndIndex=$Index+5
    export EndIndex=$EndIndex
    echo "HERE1"
    echo "Start:" $CurrentIndex
    echo "End:" $EndIndex
fi
if [ $Index = $EndIndex ]
then
    echo "HERE2" $CurrentIndex
    dataIndex=$CurrentIndex+2
    array[$dataIndex]=$DATE
    #export array[$dataIndex]=$DATE
    index=1
    fieldsCount=-4
    CIFS="$IFS"
    VAL=""
    tail $fieldsCount /tmp/pxBenchmark_outputTable.csv | while IFS=, read -a line
    do
        #if [ $index = "2" ]
        # then
        #     dataIndex=$CurrentIndex+3
        #     array[$dataIndex]=${line[1]}
        #export array[$dataIndex]=${array[$dataIndex]}
        #     echo "GPU" ${array[$dataIndex]}
        #val=${array[$dataIndex]}
        #elif [ $index = "3" ]
        #then
        #    dataIndex=$CurrentIndex+4
        #    array[$dataIndex]=${line[1]}
        #      export array[$dataIndex]=${array[$dataIndex]}
        #    echo "CPU" ${array[$dataIndex]}
        #val=${array[$dataIndex]}
        # fi
        cols+=( "${line[1]}" )
        echo "${cols[@]}"
        #if [ $dataIndex != "0" ]
        #then
        #echo "HERE3" $dataIndex $val
        # array[$dataIndex]=$val
        # export array[$dataIndex]=$val
        #fi
        dataIndex=0
        index=$index+1
    done

    echo "Cols: ${cols[@]}"
    dataIndex=$CurrentIndex+3
    #array[$dataIndex]=${cols[1]}
    echo "GPU" ${array[$dataIndex]}
    dataIndex=$CurrentIndex+4
    #array[$dataIndex]=${cols[2]}
    echo "CPU" ${array[$dataIndex]}
    IFS="$CIFS"

    CurrentIndex=-1
fi

Index=$Index+1
done

#typeset fIn="$1"
#typeset tag="td"
#VALUES=sed 's/<'"${tag}"'>\(.*\)<\/'"$tag"'>/\1/' $fIn $DATA
#VALUES=$(sed -n -e 's/.*<td>\(.*\)<\/td>.*/\1/p' <<< $DATA)
#DEVICE_NAME="Raspberry Pi";
#FIRMWARE="MODEL_3"
#var1="\<td\>$DEVICE_NAME\<\/td\>.*?\<\/tr\>"
#var2="\<td\>$DEVICE_NAME\<\/td\>\<td\>\<span style\=\"color\: rgb\(0\,0\,0\)\;\"\>$FIRMWARE\<\/span\>\<\/td\>\<td\>$DATE\<\/td\>\<td\>$GPU\<\/td\>\<td\>$CPU\<\/td\>\<td\>\$NOTES\<\/td\>\<\/tr\>"

#sed "s/$var1/$var2/g" $FORM_PATH

#sed -i 's/\"/\\"/g' $FORM_PATH
#sed -i 's/\;/\\;/g' $FORM_PATH
#sed -i 's/\./\\./g' $FORM_PATH
#sed -i 's/\,/\\,/g' $FORM_PATH
#sed -i 's/\:/\\:/g' $FORM_PATH
#data=$(cat /Users/akalok815/Desktop/viewpagestorage.txt)

#DATA=$(cat $FORM_PATH)

#curl -u akalok815:FBnl*90q*90q -X PUT -H 'Content-Type: application/json' -d '{"id":"556737708","type":"page",
#"title":"GPU+and+CPU+Benchmark+Tests","body":{"storage":{"value": "'$DATA'","representation":"storage"}}, "version":{"number":'$VERSION'}}' "https://etwiki.sys.comcast.net/##rest/api/content/556737708"

echo "DeviceName Firmware Date GPU(ms) CPU(ms) NOTES"

ARRISXI6_DN=${array[28]}
ARRISXI6_FIRMWARE="AX061AEI_VBN_1810_sprint_20181008171340sdy_PXBEN" #${array[29]}
ARRISXI6_DATE=${array[30]}
ARRISXI6_GPU=${array[31]}
ARRISXI6_CPU=${array[32]}
ARRISXI6_NOTES=${array[33]}
echo "$ARRISXI6_DN $ARRISXI6_FIRMWARE $ARRISXI6_DATE $ARRISXI6_GPU $ARRISXI6_CPU $ARRISXI6_NOTES"


ARRISXG1V4_DN=${array[35]}
ARRISXG1V4_FIRMWARE=${array[36]}
ARRISXG1V4_DATE=${array[37]}
ARRISXG1V4_GPU=${array[38]}
ARRISXG1V4_CPU=${array[39]}
ARRISXG1V4_NOTES=${array[40]}


CISCOXID_DN=${array[42]}
CISCOXID_FIRMWARE=${array[43]}
CISCOXID_DATE=${array[44]}
CISCOXID_GPU=${array[45]}
CISCOXID_CPU=${array[46]}
CISCOXID_NOTES=${array[47]}


PACEXI5_DN=${array[49]}
PACEXI5_FIRMWARE="PX051AEI_VBN_1810_sprint_20181005172529sdy_PXBEN"   #${array[50]}
PACEXI5_DATE=${array[51]}
PACEXI5_GPU=${array[52]}
PACEXI5_CPU=${array[53]}
PACEXI5_NOTES=${array[54]}

PACEXID_DN=${array[56]}
PACEXID_FIRMWARE=${array[57]}
PACEXID_DATE=${array[58]}
PACEXID_GPU=${array[59]}
PACEXID_CPU=${array[60]}
PACEXID_NOTES=${array[61]}

PACEXI3_DN=${array[63]}
PACEXI3_FIRMWARE="PX032ANI_VBN_1810_sprint_20181005174928sdy_PXBEN" #${array[64]}
PACEXI3_DATE=${array[65]}
PACEXI3_GPU=${array[66]}
PACEXI3_CPU=${array[67]}
PACEXI3_NOTES=${array[68]}

PACEXG2V2_DN=${array[70]}
PACEXG2V2_FIRMWARE=${array[71]}
PACEXG2V2_DATE=${array[72]}
PACEXG2V2_GPU=${array[73]}
PACEXG2V2_CPU=${array[74]}
PACEXG2V2_NOTES=${array[75]}

ARRISXG1V3_DN=${array[77]}
ARRISXG1V3_FIRMWARE=${array[78]}
ARRISXG1V3_DATE=${array[79]}
ARRISXG1V3_GPU=${array[80]}
ARRISXG1V3_CPU=${array[81]}
ARRISXG1V3_NOTES=${array[82]}

SAMXG2V2_DN=${array[84]}
SAMXG2V2_FIRMWARE=${array[85]}
SAMXG2V2_DATE=${array[86]}
SAMXG2V2_GPU=${array[87]}
SAMXG2V2_CPU=${array[88]}
SAMXG2V2_NOTES=${array[89]}

PACEXG1V3_DN=${array[91]}
PACEXG1V3_FIRMWARE="PX013AN_VBN_1810_sprint_20181005173211sdy_PXBEN" #${array[92]}
PACEXG1V3_DATE=${array[93]}
PACEXG1V3_GPU=${array[94]}
PACEXG1V3_CPU=${array[95]}
PACEXG1V3_NOTES=${array[96]}

CISCOG8_DN=${array[98]}
CISCOG8_FIRMWARE=${array[99]}
CISCOG8_DATE=${array[100]}
CISCOG8_GPU=${array[101]}
CISCOG8_CPU=${array[102]}
CISCOG8_NOTES=${array[103]}

PACEXG1V1_DN=${array[105]}
PACEXG1V1_FIRMWARE=${array[106]}
PACEXG1V1_DATE=${array[107]}
PACEXG1V1_GPU=${array[108]}
PACEXG1V1_CPU=${array[109]}
PACEXG1V1_NOTES=${array[110]}

ARRISXG1V1_DN=${array[112]}
ARRISXG1V1_FIRMWARE=${array[113]}
ARRISXG1V1_DATE=${array[114]}
ARRISXG1V1_GPU=${array[115]}
ARRISXG1V1_CPU=${array[116]}
ARRISXG1V1_NOTES=${array[117]}
echo "$ARRISXG1V1_DN $ARRISXG1V1_FIRMWARE $ARRISXG1V1_DATE $ARRISXG1V1_GPU $ARRISXG1V1_CPU $ARRISXG1V1_NOTES"

RSP_DN=${array[119]}
RSP_FIRMWARE=${array[120]}
RSP_DATE=${array[121]}
RSP_GPU=${array[122]}
RSP_CPU=${array[123]}
RSP_NOTES=${array[124]}
echo "$RSP_DN $RSP_FIRMWARE $RSP_DATE $RSP_CPU $RSP_GPU $RSP_NOTES"


echo "PACEXI3_FIRMWARE:" $PACEXI3_FIRMWARE
# json directly in bash script
# ref: https://stackoverflow.com/questions/43373176/store-json-directly-in-bash-script-with-variables/43373520
# https://superuser.com/questions/284187/bash-iterating-over-lines-in-a-variable
html_body=$(cat <<EOF
<p>The goal below is to benchmark the GPU and CPU on X1 devices and Raspberry PI by using Spark*s APIs.&nbsp; We are
not validating the feature sets but comparing performance across the devices.</p>
<p><br /></p>
<h1>pxBenchmark</h1>
<p><a href="https://github.com/pxscene/pxCore/tree/master/examples/pxBenchmark">https://github.com/pxscene/pxCore/tree/master/examples/pxBenchmark</a></p>
<p>These results were collected using pxBenchmark.&nbsp; pxBenchmark is an application that is used to benchmark the
GPU and CPU performance on RDK devices.&nbsp; It uses Spark APIs to perform the GPU and CPU benchmark tests.&nbsp;
The GPU results are the values of the total time spent inside pxContext APIs.&nbsp; Currently, the tests used for
CPU collections are JPEG and PNG image decoding.</p>
<p><br /></p>
<h1>Benchmark Results</h1>
<p><br /></p>
<table class="wrapped">
<colgroup>
<col style="width: 124.0px;" />
<col style="width: 84.0px;" />
<col style="width: 83.0px;" />
<col style="width: 159.0px;" />
<col style="width: 158.0px;" />
<col style="width: 60.0px;" />
</colgroup>
<tbody>
<tr>
<th>Device Type</th>
<th>Firmware</th>
<th>Date Run</th>
<th>
<p>GPU Execution Time</p>
<p>Unit: ms</p>
</th>
<th>
<p>CPU Execution Time</p>
<p>Unit: ms</p>
</th>
<th>Notes</th>
</tr>
<tr>
<td><span>$ARRISXI6_DN</span></td>
<td><span style="color: rgb(0,0,0);">$ARRISXI6_FIRMWARE<br /></span></td>
<td>$ARRISXI6_DATE</td>
<td><span>$ARRISXI6_GPU</span></td>
<td>$ARRISXI6_CPU</td>
<td>$ARRISXI6_NOTES</td>
</tr>
<tr>
<td>$ARRISXG1V4_DN</td>
<td><span style="color: rgb(0,0,0);">$ARRISXG1V4_FIRMWARE<br /></span></td>
<td>$ARRISXG1V4_DATE</td>
<td>$ARRISXG1V4_GPU</td>
<td><span>$ARRISXG1V4_CPU</span></td>
<td>$ARRISXG1V4_NOTES</td>
</tr>
<tr>
<td>$CISCOXID_DN</td>
<td><span style="color: rgb(0,0,0);">$CISCOXID_FIRMWARE<br /></span></td>
<td><span>$CISCOXID_DATE</span></td>
<td>$CISCOXID_GPU</td>
<td><span>$CISCOXID_CPU</span></td>
<td>$CISCOXID_NOTES</td>
</tr>
<tr>
<td>$PACEXI5_DN</td>
<td><span style="color: rgb(0,0,0);">$PACEXI5_FIRMWARE<br /></span></td>
<td><span>$PACEXI5_DATE</span></td>
<td>$PACEXI5_GPU</td>
<td><span>$PACEXI5_CPU</span></td>
<td>$PACEXI5_NOTES</td>
</tr>
<tr>
<td>$PACEXID_DN</td>
<td><span style="color: rgb(0,0,0);">$PACEXID_FIRMWARE<br /></span></td>
<td><span>$PACEXID_DATE</span></td>
<td>$PACEXID_GPU</td>
<td><span>$PACEXID_CPU</span></td>
<td>$PACEXID_NOTES</td>
</tr>
<tr>
<td>$PACEXI3_DN</td>
<td><span style="color: rgb(0,0,0);">$PACEXI3_FIRMWARE<br /></span></td>
<td><span>$PACEXI3_DATE</span></td>
<td>$PACEXI3_GPU</td>
<td><span>$PACEXI3_CPU</span></td>
<td>$PACEXI3_NOTES</td>
</tr>
<tr>
<td>$PACEXG2V2_DN</td>
<td><span style="color: rgb(0,0,0);">$PACEXG2V2_FIRMWARE<br /></span></td>
<td><span>$PACEXG2V2_DATE</span></td>
<td>$PACEXG2V2_GPU</td>
<td><span>$PACEXG2V2_CPU</span></td>
<td>$PACEXG2V2_NOTES</td>
</tr>
<tr>
<td>$ARRISXG1V3_DN</td>
<td><span style="color: rgb(0,0,0);">$ARRISXG1V3_FIRMWARE<br /></span></td>
<td><span>$ARRISXG1V3_DATE</span></td>
<td>$ARRISXG1V3_GPU</td>
<td><span>$ARRISXG1V3_CPU</span></td>
<td>$ARRISXG1V3_NOTES</td>
</tr>
<tr>
<td>$SAMXG2V2_DN</td>
<td><span style="color: rgb(0,0,0);">$SAMXG2V2_FIRMWARE<br /></span></td>
<td><span>$SAMXG2V2_DATE</span></td>
<td>$SAMXG2V2_GPU</td>
<td><span>$SAMXG2V2_CPU</span></td>
<td>$SAMXG2V2_NOTES</td>
</tr>
<tr>
<td>$PACEXG1V3_DN</td>
<td><span style="color: rgb(0,0,0);">$PACEXG1V3_FIRMWARE<br /></span></td>
<td><span>$PACEXG1V3_DATE</span></td>
<td>$PACEXG1V3_GPU</td>
<td><span>$PACEXG1V3_CPU</span></td>
<td>$PACEXG1V3_NOTES</td>
</tr>
<tr>
<td>$CISCOG8_DN</td>
<td><span style="color: rgb(0,0,0);">$CISCOG8_FIRMWARE<br /></span></td>
<td><span>$CISCOG8_DATE</span></td>
<td>$CISCOG8_GPU</td>
<td><span>$CISCOG8_CPU</span></td>
<td>$CISCOG8_NOTES</td>
</tr>
<tr>
<td>$PACEXG1V1_DN</td>
<td><span style="color: rgb(0,0,0);">$PACEXG1V1_FIRMWARE<br /></span></td>
<td><span>$PACEXG1V1_DATE</span></td>
<td>$PACEXG1V1_GPU</td>
<td><span>$PACEXG1V1_CPU</span></td>
<td>$PACEXG1V1_NOTES</td>
</tr>
<tr>
<td>$ARRISXG1V1_DN</td>
<td><span style="color: rgb(0,0,0);">$ARRISXG1V1_FIRMWARE<br /></span></td>
<td><span>$ARRISXG1V1_DATE</span></td>
<td>$ARRISXG1V1_GPU</td>
<td><span>$ARRISXG1V1_CPU</span></td>
<td>$ARRISXG1V1_NOTES</td>
</tr>
<tr>
<td>$RSP_DN</td>
<td><span style="color: rgb(0,0,0);">$RSP_FIRMWARE<br /></span></td>
<td><span>$RSP_DATE</span></td>
<td><span>$RSP_GPU</span></td>
<td><span>$RSP_CPU</span></td>
<td>
<ac:task-list>
<ac:task>
<ac:task-id>1</ac:task-id>
<ac:task-status>incomplete</ac:task-status>
<ac:task-body><span>&nbsp;</span></ac:task-body>
</ac:task>
</ac:task-list>
</td>
</tr>
</tbody>
</table>
EOF
)

echo "==========================="
echo $html_body
echo "==========================="
html_body1=${html_body//\"/\\\"}
idx=0
html_body2=""
while read -r line; do
if [[ ${idx} -ne 0 ]]; then
html_body2+="\n"
fi

html_body2+="$line"
idx+=1
done <<< "$html_body1"
echo "==========================="
echo $html_body2
echo "==========================="

payload=$(cat <<EOF
{
"id":"556737708",
"type":"page",
"title":"GPU+and+CPU+Benchmark+Tests",
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
echo $payload
echo "==========================="

curl -u akalok815:FBnl*90q*90q -X PUT -H 'Content-Type: application/json' "https://etwiki.sys.comcast.net/rest/api/content/556737708" --data "$payload"



#for token in "${array[@]}"
#do
#echo -n "$token"
#done

IFS="$OIFS"
