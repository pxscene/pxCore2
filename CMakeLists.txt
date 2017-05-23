cmake_minimum_required(VERSION 2.8)
project(pxscene2d)

option(BUILD_PX_TESTS "BUILD_PX_TESTS" OFF)

add_subdirectory(src)
add_subdirectory(examples/pxScene2d/src)

if (BUILD_PX_TESTS)
    message("Including unit test support")
    add_subdirectory(tests/pxScene2d)
endif (BUILD_PX_TESTS)
