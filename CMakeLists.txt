cmake_minimum_required(VERSION 2.8)
project(pxscene2d)

option(BUILD_PXCORE "BUILD_PXCORE" ON)
option(BUILD_PXSCENE "BUILD_PXSCENE" ON)
option(BUILD_PX_TESTS "BUILD_PX_TESTS" OFF)
option(BUILD_RTREMOTE_LIBS "BUILD_RTREMOTE_LIBS" OFF)

if (BUILD_PXCORE)
    message("Building pxcore")
    add_subdirectory(src)
endif (BUILD_PXCORE)

if (BUILD_RTREMOTE_LIBS)
    message("Building rtRemote")
    add_subdirectory(remote)
endif (BUILD_RTREMOTE_LIBS)

if (BUILD_PXSCENE)
    message("Building pxscene")
    add_subdirectory(examples/pxScene2d/src)
endif (BUILD_PXSCENE)

if (BUILD_PX_TESTS)
    message("Including unit test support")
    add_subdirectory(tests/pxScene2d)
endif (BUILD_PX_TESTS)
