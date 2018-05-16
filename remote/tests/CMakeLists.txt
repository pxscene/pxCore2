cmake_minimum_required(VERSION 2.8)
project(rtUnitTest)

set(PXCOREDIR ${CMAKE_CURRENT_SOURCE_DIR}/../../)
set(EXTDIR "${PXCOREDIR}/examples/pxScene2d/external")
set(RTREMOTEINC ${PXCOREDIR}/remote)
set(PXCORESRCDIR ${PXCOREDIR}/src)
set(GOOGLETESTINC ${EXTDIR}/gtest/googletest/include ${EXTDIR}/gtest/googletest ${EXTDIR}/gtest/googlemock/include ${EXTDIR}/gtest/googlemock)
set(TEST_SOURCE_FILES
    rtRemoteEnvironmentTest.cpp
    rtRemoteTest.cpp
    rtRpcUnitTest.cpp
    rtConnectionTest.cpp
    rtRemoteMessageTest.cpp
    rtRemoteEndPointTest.cpp
    rtRemoteFactoryTest.cpp)

set(GOOGLE_TEST_SOURCE_FILES ${EXTDIR}/gtest/googletest/src/gtest_main.cc ${EXTDIR}/gtest/googletest/src/gtest-all.cc ${EXTDIR}/gtest/googlemock/src/gmock-all.cc)

set(TEST_SOURCE_FILES ${TEST_SOURCE_FILES} ${GOOGLE_TEST_SOURCE_FILES})

set(TEST_SOURCE_FILES_SERVER rtRemoteServerTest.cpp ${GOOGLE_TEST_SOURCE_FILES})

set(CLIENT_SOURCE_FOR_SERVER_TEST client_for_server_test.cpp)

include_directories(AFTER ${GOOGLETESTINC} ${RTREMOTEINC} ${PXCORESRCDIR})
link_directories(${RTREMOTEINC} ${PXCOREDIR}/build/glut ${PXCOREDIR}/build/egl)

set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}  -g -O0 -fno-inline -fprofile-arcs -ftest-coverage --coverage")

file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/lcov.sh" DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/build")
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/../rtremote.conf" DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/build")
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/../rtremote.conf.unix" DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/build")

add_executable(rtUnitTest ${TEST_SOURCE_FILES})
target_link_libraries(rtUnitTest -lrtRemote -lrtCore -pthread -coverage -std=c++11 -lm)
target_compile_definitions(rtUnitTest PRIVATE RT_PLATFORM_LINUX RAPIDJSON_HAS_STDSTRING)

add_executable(rtUnitTestServer ${TEST_SOURCE_FILES_SERVER})
target_link_libraries(rtUnitTestServer -lrtRemote -lrtCore -pthread -coverage -std=c++11 -lm)
target_compile_definitions(rtUnitTestServer PRIVATE RT_PLATFORM_LINUX RAPIDJSON_HAS_STDSTRING)

add_executable(client_for_server_test ${CLIENT_SOURCE_FOR_SERVER_TEST})
target_link_libraries(client_for_server_test -lrtRemote -lrtCore -pthread -coverage -std=c++11 -lm)
target_compile_definitions(client_for_server_test PRIVATE RT_PLATFORM_LINUX RAPIDJSON_HAS_STDSTRING)
