cmake_minimum_required(VERSION 2.8)
project(pxscene2d)

option(BUILD_PXCORE "BUILD_PXCORE" ON)
option(BUILD_PXSCENE "BUILD_PXSCENE" ON)
option(BUILD_PX_TESTS "BUILD_PX_TESTS" OFF)
option(BUILD_RTREMOTE_LIBS "BUILD_RTREMOTE_LIBS" OFF)
option(BUILD_PXBENCHMARK "BUILD_PXBENCHMARK" OFF)

option(PREFER_SYSTEM_LIBRARIES "Prefer to use system libraries over bundled." "OFF")
    # Fine selection of which external node package to pick-up, if multiple are installed on the system
    option(PKG_CONFIG_DISABLE_NODE  "When PREFER_SYSTEM_LIBRARIES=ON, it disables searching for 'node' pkg-config"   OFF)
    option(PKG_CONFIG_DISABLE_NODE8 "When PREFER_SYSTEM_LIBRARIES=ON, it disables searching for 'node8' pkg-config"  OFF)

option(DISABLE_DEBUG_MODE "Disable debugging mode." "OFF") # OFF for backward compatibility

option(SUPPORT_NODE "SUPPORT_NODE" ON)
option(SUPPORT_V8 "SUPPORT_V8" OFF)
option(SUPPORT_DUKTAPE "SUPPORT_DUKTAPE" ON)

option(ENABLE_THREAD_SANITIZER "ENABLE_THREAD_SANITIZER" OFF)
option(ENABLE_ADDRESS_SANITIZER "ENABLE_ADDRESS_SANITIZER" OFF)

if (ENABLE_THREAD_SANITIZER)
    message("Enabling thread sanitizer")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=thread")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread")
endif (ENABLE_THREAD_SANITIZER)

if (ENABLE_ADDRESS_SANITIZER)
    message("Enabling address sanitizer")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
endif (ENABLE_ADDRESS_SANITIZER)

if (BUILD_DUKTAPE)
    message("Building duktape with pxcore")
    add_subdirectory(examples/pxScene2d/external/dukluv)
endif (BUILD_DUKTAPE)

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

if (BUILD_PXBENCHMARK)
    message("Building pxbenchmark")
    add_subdirectory(examples/pxBenchmark/src)
endif (BUILD_PXBENCHMARK)
