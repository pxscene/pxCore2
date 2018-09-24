#!/bin/sh
entryfile=""
outputfile="index.js"
enableJar=0
parsinginput=0
inputpassed=0

#read the options passed in command line
while getopts ":e:o:j:h" opt; do
  case $opt in
    e) entryfile="$OPTARG"; inputpassed=1;
    ;;
    o) outputfile="$OPTARG"
    ;;
    j) enableJar="$OPTARG"
    ;;
    h) echo "Usage : ./tool.sh -e <entry filename> -o <output filename(optional)> -j 1/0(enable/disable jar)" >&2; exit 1;
    ;;
    \?) echo "Invalid option -$OPTARG"; echo "Usage : ./tool.sh -e <entry filename> -o <output filename(optional)> -j 1/0(enable/disable jar)" >&2; >&2; exit 1;
    ;;
  esac
done

if [ "$inputpassed" -eq 0 ]; then
  echo "Entry file not provided"
  exit 1;
fi

#run webpack and get the output file with the name user mentioned
webpack -o "dist/$outputfile"

#sed -i 's/function(module, exports, __webpack_require__) {/function(module, exports, __webpack_require__) {\n px.registerCode(__webpack_require__.m)\n/g' dist/$outputfile

#remove all the lines where module.exports is provided, as spark have own module
linenumbers=`grep -rn "^module.exports = __webpack_require__" dist/$outputfile|grep -v "./pack/$entryfile" | awk -F: '{print $1}'`
delcmd="sed -i '"
for number in $linenumbers
do
delcmd=$delcmd" $number d;"
done
delcmd=$delcmd"' dist/$outputfile"
eval "$delcmd"

#remove all the lines where __webpack_require__ is done with all the files execpt the main file
linenumbers=`grep -rn "^__webpack_require__(" dist/$outputfile|grep -v "./pack/$entryfile"|awk -F: '{print $1}'`
delcmd="sed -i '"
for number in $linenumbers
do
delcmd=$delcmd" $number d;"
done
delcmd=$delcmd"' dist/$outputfile"
eval "$delcmd"

#replace all the lines starting with format ./pack/filename to filename
sed -i 's/\.\/pack\///g' dist/$outputfile

#replace any module.exports line with index.js
sed -i "s/^module.exports = __webpack_require__(\/\*! $entryfile \*\/\\\"$entryfile\\\");/__webpack_require__(\/\*! $entryfile \*\/\\\"$entryfile\\\");/g" dist/$outputfile

#perform registration of source code on the start of first main file to px layer
sed -i "s/__webpack_require__(\/\*! $entryfile \*\/\\\"$entryfile\\\");/px.registerCode(__webpack_require__.m)\n__webpack_require__(\/\*! $entryfile \*\/\\\"$entryfile\\\");/g" dist/$outputfile

#replace all the module,exports variables of webpack to module,exports provided from spark, so it is visible inside spark
sed -i 's/function(module, exports)/function(module=px.module, exports=px.exports)/g' dist/$outputfile

mv dist/pack/* dist/.
rm -rf dist/pack
cp FreeSans.ttf dist/.

if [ "$enableJar" -eq 1 ]; then
  echo "{\"main\" : \"$outputfile\"}" > dist/package.json
  cd dist
  jar cvf bundle.jar *
  echo "Output bundle.jar generated within dist !!!!!!!!!!!!!"
else
  echo "Output file $outputfile generated within dist !!!!!!!!!!!!!"
fi

