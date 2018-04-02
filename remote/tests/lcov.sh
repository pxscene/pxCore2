#!/bin/bash
# this shell script will be used to generate code coverage report
rm -rf testResult*
rm -rf result

lcov -c -d ../../ -c -o tracefile
lcov --remove tracefile '/usr/include/*' '*src/*' '*/remote/rapidjson/*' '*/remote/tests/*' '*external*' '*Applications*' -o tracefile_rtremote
rm -fr tracefile

lcov -c -d ../../obj -c -o tracefile
lcov --remove tracefile '/usr/include/*' '*src/*' '*/remote/rapidjson/*' '*/remote/tests/*' '*external*' '*Applications*' -o tracefile_rtremote_obj
rm -fr tracefile

lcov -a tracefile_rtremote -a tracefile_rtremote_obj -o testResult.info

genhtml -o result testResult.info

rm -fr tracefile_rtremote tracefile_rtremote_obj

