cd $TRAVIS_BUILD_DIR
rm -rf reports
rm -rf tracefile

lcov -d temp/src/CMakeFiles/rtCore_s.dir -c -o tracefile
lcov --remove tracefile '/usr/include/*' '*external*' '*Applications*' -o tracefile_rtcore_static
rm -rf tracefile

lcov -d temp/src/CMakeFiles/pxCore.dir -c -o tracefile
lcov --remove tracefile '/usr/include/*' '*external*' '*Applications*' -o tracefile_pxcore
rm -rf tracefile

lcov -c -d temp/examples/pxScene2d/src/CMakeFiles/pxscene_app.dir -o tracefile
lcov --remove tracefile '/usr/include/*' '*external*' '*Applications*' -o tracefile_pxscene_app
rm -rf tracefile

lcov -c -d temp/examples/pxScene2d/src/CMakeFiles/pxscene_static.dir -o tracefile
lcov --remove tracefile '/usr/include/*' '*external*' '*Applications*' -o tracefile_pxscene_static
rm -rf tracefile

lcov -c -d temp/tests/pxScene2d/CMakeFiles/pxscene2dtests.dir -o tracefile
lcov --remove tracefile '*tests*' '/usr/include/*' '*external*' '*Applications*' -o tracefile_ut
rm -rf tracefile

lcov -a tracefile_rtcore_static -a tracefile_pxcore -a tracefile_pxscene_app -a tracefile_pxscene_static -a tracefile_ut -o tracefile

if [ "$#" -ne  "0" ]
then
if [ "$1" == "--gen-reports" ]
then
mkdir reports
genhtml -o reports tracefile
fi
fi
rm -rf tracefile_rtcore_static tracefile_pxcore tracefile_pxscene_app tracefile_pxscene_static
