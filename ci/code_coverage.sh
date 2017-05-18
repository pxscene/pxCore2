currentdir="$(pwd)"
cd src
lcov -c -d obj/ -o tracefile
lcov --remove tracefile '/usr/include/*' '*external*' -o tracefile_pxcore
rm -rf tracefile

cd ../examples/pxScene2d/src
lcov -c -d obj/ -o tracefile
lcov --remove tracefile '/usr/include/*' '*external*' -o tracefile_pxscene
rm -rf tracefile

cd $currentdir
lcov -a src/tracefile_pxcore -a examples/pxScene2d/src/tracefile_pxscene -o tracefile
if [ "$#" -ne  "0" ]
then
if [ "$1" == "--gen-reports" ]
then
mkdir reports
genhtml -o reports tracefile
fi
fi
rm -rf src/tracefile_pxcore examples/pxScene2d/src/tracefile_pxscene
