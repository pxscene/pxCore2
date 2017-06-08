cd $TRAVIS_BUILD_DIR

rm -rf reports
rm -rf tracefile

lcov -d ./build/pxCore.build/Debug/pxCore\ Static\ Library.build/Objects-normal/x86_64/ -c -o tracefile
lcov --remove tracefile '/usr/include/*' '*external*' '*Applications*' -o tracefile_pxcore
rm -rf tracefile

lcov -c -d examples/pxScene2d/src/obj/ -o tracefile
lcov --remove tracefile '/usr/include/*' '*external*' '*Applications*' -o tracefile_pxscene
rm -rf tracefile

lcov -a tracefile_pxcore -a tracefile_pxscene -o tracefile

if [ "$#" -ne  "0" ]
then
if [ "$1" == "--gen-reports" ]
then
mkdir reports
genhtml -o reports tracefile
fi
fi
rm -rf tracefile_pxcore tracefile_pxscene
