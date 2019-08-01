#!/bin/bash

TESTS_DIR="${TRAVIS_BUILD_DIR}/tests/pxScene2d"
TESTSDESKTOP_JSON="${TESTS_DIR}/testRunner/testsDesktop.json"
cp "${TESTSDESKTOP_JSON}" "${TESTS_DIR}/testRunner/testsDesktop.json_orig"

TESTS_JSON="${TESTS_DIR}/testRunner/tests.json"
cp "${TESTS_JSON}" "${TESTS_DIR}/testRunner/tests.json_orig"

drop_words()
{
  MATCH=$1
  FILE=$2
  if [ "$TRAVIS_OS_NAME" = "osx" ]; then
    sed -i -n "s/${MATCH}//g" "${FILE}"
  else
    sed -i "s/${MATCH}//g" "${FILE}"
  fi
}

drop_json_lines()
{
  MATCH=$1
  FILE=$2
  if [ "$TRAVIS_OS_NAME" = "osx" ]; then
    sed -i -n "/${MATCH}/d" "${FILE}"
  else
    sed -i "/${MATCH}/d" "${FILE}"
  fi
  #delete last comma in json file, if any
  sed -i -e x -e '$ {s/,$//;p;x;}' -e 1d "${FILE}"
}

#make arrangements for ignoring some tests
drop_json_lines "pxWayland" "${TESTSDESKTOP_JSON}"
if ! grep -q "TEST_PERMISSIONS_CHECK\" ON" "${TESTS_DIR}/CMakeLists.txt" ; then
  drop_json_lines "permissions" "${TESTS_JSON}"
fi
if ! grep -q "ACCESS_CONTROL_CHECK\" ON" "${TRAVIS_BUILD_DIR}/examples/pxScene2d/src/CMakeLists.txt"; then
  drop_json_lines "cors" "${TESTS_JSON}"
fi

if [ "$TRAVIS_EVENT_TYPE" = "cron" ] && [ "$TRAVIS_JOB_NAME" = "duktape_validation" ]
then
  ignoretests=( "test_simpleApiParent" "test_xre2_1555" "test_pxAnimateTimeUnits" "test_pxAnimate" "test_imports" "test_permissions" "test_permissions_http2" "test_cors" "test_cors_http2" "test_promiseRejectionAnimation" "test_promiseRejectionImage" "test_promiseRejectionReload" "test_xre2-1658" "test_moduleApi" "test_screencapture_jar" "test_pxColorNames" "test_dirty_rectangles" )
  for i in "${ignoretests[@]}"
  do
    drop_json_lines "$i" "${TESTS_JSON}"
  done

  ignoreunittests=( "test_jsfiles.cpp" "test_httpEndAfterClose.cpp" "test_memoryleak.cpp" "test_rtnode.cpp" "test_eventListeners.cpp" "test_rtObjectWrapper.cpp"  )
  for i in "${ignoreunittests[@]}"
  do
    drop_words "$i" "${TESTS_DIR}/CMakeLists.txt"
  done

  if [ "$TRAVIS_OS_NAME" = "osx" ]; then
    sed -i -n '/cd supportfiles/i ln \-s \.\.\/\.\.\/examples\/pxScene2d\/src\/duk_modules\/\ duk_modules' "${TESTS_DIR}/pxscene2dtests.sh"
    sed -i -n '/rm\ \-rf\ node_modules/i rm\ \-rf\ duk_modules' "${TESTS_DIR}/pxscene2dtests.sh"
  else
    sed -i '/cd supportfiles/i ln \-s \.\.\/\.\.\/examples\/pxScene2d\/src\/duk_modules\/\ duk_modules' "${TESTS_DIR}/pxscene2dtests.sh"
    sed -i '/rm\ \-rf\ node_modules/i rm\ \-rf\ duk_modules' "${TESTS_DIR}/pxscene2dtests.sh"
  fi
fi

exit 0;
