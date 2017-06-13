cmake_minimum_required(VERSION 2.8)
project(pxscene2dtests)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

set(PXCOREDIR ${CMAKE_CURRENT_SOURCE_DIR}/../../)
set(EXTDIR "${PXCOREDIR}/examples/pxScene2d/external")
set(NODEDIR "${EXTDIR}/libnode-v6.9.0/")
set(NODEINC ${NODEDIR}/src ${NODEDIR}/deps/uv/include ${NODEDIR}/deps/v8 ${NODEDIR}/deps/debugger-agent/include ${NODEDIR}/deps/v8/include ${NODEDIR}/deps/cares/include)
set(ZLIBINC "${EXTDIR}/zlib/")
set(CURLINC "${EXTDIR}/curl/include")
set(JPGINC "${EXTDIR}/jpg")
set(JPGTURBOINC "${EXTDIR}/libjpeg-turbo")
set(PNGINC "${EXTDIR}/png")
set(FREETYPEINC "${EXTDIR}/ft/include")
set(WESTEROSINC ${EXTDIR}/westeros/external/install/include)
set(WESTEROSSTUBINC ${EXTDIR}/westeros-stub)
set(JPEGTURBOINC ${EXTDIR}/libjpeg-turbo)
set(BREAKPADINC ${EXTDIR}/breakpad/src)
set(RTREMOTEINC ${PXCOREDIR}/remote)
set(GOOGLETESTINC ${EXTDIR}/gtest/googletest/include ${EXTDIR}/gtest/googletest ${EXTDIR}/gtest/googlemock/include ${EXTDIR}/gtest/googlemock)
set(PXCOREINC ${PXCOREDIR}/src)
set(PXSCENEINC ${PXCOREDIR}/examples/pxScene2d/src)

set(BUILD_UNIT_TEST 0)


include_directories(AFTER ${ZLIBINC} ${CURLINC} ${JPGINC} ${JPGTURBOINC} ${PNGINC} ${GLUTINC} ${FREETYPEINC} ${GOOGLETESTINC} ${PXCOREINC}
        ${PXSCENEINC} ${NODEINC})
include_directories(AFTER ${CMAKE_CURRENT_SOURCE_DIR})

if (CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
    if (HOSTNAME STREQUAL "raspberrypi")
        message("Unit tests are not currently supported on raspberry pi")
    else ()
        message("Building unit tests for glut")
        set(PX_PLATFORM PX_PLATFORM_GLUT)
        set(PXCORE_LIB_LOCATION ${PXCOREDIR}/build/glut)
        set(PXSCENETEST_LINK_DIRECTORIES ${PXCORE_LIB_LOCATION})
        set(PLATFORM_LIBRARIES pxscene node pxCore rtCore_s pthread v8_libplatform png16 X11 glut GLEW GL dl rt turbojpeg jpeg freetype curl z)
        add_definitions(-DRT_PLATFORM_LINUX -DENABLE_NODE_V_6_9 -DNODE_PLATFORM="linux" -DNODE_WANT_INTERNALS=1 -DV8_DEPRECATION_WARNINGS=1 -DNODE_SHARED_MODE -DNODE_USE_V8_PLATFORM=1 -DNODE_HAVE_I18N_SUPPORT=1 -DNODE_HAVE_SMALL_ICU=1 -DHAVE_INSPECTOR=1 -DV8_INSPECTOR_USE_STL=1 -DV8_INSPECTOR_USE_OLD_STL=1 -DENABLE_DEBUG_MODE -DENABLE_LIBJPEG_TURBO -DPNG_APNG_SUPPORTED)
        set(BUILD_UNIT_TEST 1)
    endif (HOSTNAME STREQUAL "raspberrypi")
else ()
    message("Unit tests are not currently supported on this platform")
endif (CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")

add_definitions(-D${PX_PLATFORM} -DENABLE_RT_NODE -DRUNINMAIN -DENABLE_HTTP_CACHE)

set(TEST_SOURCE_FILES pxscene2dtestsmain.cpp test_example.cpp test_api.cpp test_pxcontext.cpp test_memoryleak.cpp test_rtnode.cpp
        test_pxAnimate.cpp test_jsfiles.cpp test_pxWayland.cpp test_pxWaylandContainer.cpp)

if (DEFINED ENV{USE_HTTP_CACHE})
    message("Include http cache tests")
    set(TEST_SOURCE_FILES ${TEST_SOURCE_FILES} test_imagecache.cpp)
endif (DEFINED ENV{USE_HTTP_CACHE})

set(TEST_SOURCE_FILES ${TEST_SOURCE_FILES} ${EXTDIR}/gtest/googletest/src/gtest-all.cc ${EXTDIR}/gtest/googlemock/src/gmock-all.cc)

set(PXSCENE_TEST_LINK_LIBRARIES ${PLATFORM_LIBRARIES})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -fpermissive -g -Wall -Wno-attributes -Wall -Wextra -Wno-write-strings -Wno-format-security -std=c++11 -O3")

set(PXSCENETEST_LINK_DIRECTORIES ${PXSCENETEST_LINK_DIRECTORIES} ${EXTDIR}/png/.libs)
set(PXSCENETEST_LINK_DIRECTORIES ${PXSCENETEST_LINK_DIRECTORIES} ${EXTDIR}/jpg/.libs)
set(PXSCENETEST_LINK_DIRECTORIES ${PXSCENETEST_LINK_DIRECTORIES} ${EXTDIR}/ft/objs/.libs)
set(PXSCENETEST_LINK_DIRECTORIES ${PXSCENETEST_LINK_DIRECTORIES} ${EXTDIR}/curl/lib/.libs)
set(PXSCENETEST_LINK_DIRECTORIES ${PXSCENETEST_LINK_DIRECTORIES} ${EXTDIR}/zlib)
set(PXSCENETEST_LINK_DIRECTORIES ${PXSCENETEST_LINK_DIRECTORIES} ${NODEDIR}/out/Release/obj.target ${NODEDIR}/out/Release ${NODEDIR}/out/Release/obj.target/deps/v8/tools/gyp)
set(PXSCENETEST_LINK_DIRECTORIES ${PXSCENETEST_LINK_DIRECTORIES} ${EXTDIR}/libjpeg-turbo/.libs)
set(PXSCENETEST_LINK_DIRECTORIES ${PXSCENETEST_LINK_DIRECTORIES} ${PXSCENEINC})

if (BUILD_UNIT_TEST GREATER 0)
    link_directories(${PXSCENETEST_LINK_DIRECTORIES})
    add_executable(pxscene2dtests ${TEST_SOURCE_FILES})
    target_link_libraries(pxscene2dtests ${PXSCENE_TEST_LINK_LIBRARIES})
endif (BUILD_UNIT_TEST GREATER 0)
