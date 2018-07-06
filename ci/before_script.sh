#!/bin/sh

TESTS_DIR=$TRAVIS_BUILD_DIR/tests/pxScene2d
TESTS_JSON="${TESTS_DIR}/testRunner/tests.json"
cp "${TESTS_JSON}" "${TESTS_DIR}/testRunner/tests.json_orig"

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
drop_json_lines "pxWayland" "${TESTS_JSON}"
if ! grep -q "TEST_PERMISSIONS_CHECK\" ON" "${TESTS_DIR}/CMakeLists.txt" ; then
  drop_json_lines "permissions" "${TESTS_JSON}"
fi
if ! grep -q "TEST_ACCESS_CONTROL_CHECK\" ON" "${TESTS_DIR}/CMakeLists.txt"; then
  drop_json_lines "cors" "${TESTS_JSON}"
fi

exit 0;
