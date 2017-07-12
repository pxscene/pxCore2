cd $TRAVIS_BUILD_DIR
find . -name *.gcda
find . -name *.gcno
echo "gcda printed"

rm -rf reports
rm -rf tracefile

pwd
ls -lrt src/obj
ls -lrt examples/pxScene2d/src/obj/

lcov -c -d src/obj/ -o tracefile
lcov --remove tracefile '/usr/include/*' '*external*' -o tracefile_pxcore
rm -rf tracefile

lcov -c -d examples/pxScene2d/src/obj/ -o tracefile
lcov --remove tracefile '/usr/include/*' '*external*' -o tracefile_pxscene
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
