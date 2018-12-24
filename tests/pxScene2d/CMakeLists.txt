cmake_minimum_required(VERSION 2.8)
project(pxscene2dtests)
find_package(PkgConfig)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

set(PXCOREDIR ${CMAKE_CURRENT_SOURCE_DIR}/../../)
set(EXTDIR "${PXCOREDIR}/examples/pxScene2d/external")
set(UWS ${EXTDIR}/uWebSockets)

include(../../cmake/CommOpts.cmake)
include(../../cmake/CommDeps.cmake)
include(../../cmake/NodeDeps.cmake)
include(../../cmake/DukeDeps.cmake)

set(WESTEROSINC ${EXTDIR}/westeros/external/install/include)
set(WESTEROSSTUBINC ${EXTDIR}/westeros-stub)
set(RTREMOTEINC ${PXCOREDIR}/remote)
set(GOOGLETESTINC ${EXTDIR}/gtest/googletest/include ${EXTDIR}/gtest/googletest ${EXTDIR}/gtest/googlemock/include ${EXTDIR}/gtest/googlemock)
set(PXCOREINC ${PXCOREDIR}/src)
set(PXSCENEINC ${PXCOREDIR}/examples/pxScene2d/src)
set(PXSCENERASTERINC ${PXCOREDIR}/examples/pxScene2d/src/rasterizer)
set(PXSCENELIBDIR ${PXCOREDIR}/examples/pxScene2d/src)

set(BUILD_UNIT_TEST 0)

option(BUILD_WITH_WINDOWLESS_EGL "BUILD_WITH_WINDOWLESS_EGL" OFF)
option(PXSCENE_TEST_HTTP_CACHE "PXSCENE_TEST_HTTP_CACHE" OFF)
option(PXSCENE_TEST_PERMISSIONS_CHECK "PXSCENE_TEST_PERMISSIONS_CHECK" ON)


include_directories(AFTER ${GOOGLETESTINC} ${PXCOREINC} ${PXSCENEINC} ${PXSCENERASTERINC})
include_directories(AFTER ${CMAKE_CURRENT_SOURCE_DIR})

add_definitions(${COMM_DEPS_DEFINITIONS})
include_directories(AFTER
            ${NODE_INCLUDE_DIRS}
            ${DUKE_INCLUDE_DIRS}
       ${COMM_DEPS_INCLUDE_DIRS}
              ${WESTEROSSTUBINC}
     ${WAYLAND_EGL_INCLUDE_DIRS}
  ${WAYLAND_CLIENT_INCLUDE_DIRS}
)

set(PXSCENETEST_LINK_DIRECTORIES ${PXSCENETEST_LINK_DIRECTORIES}
            ${NODE_LIBRARY_DIRS}
            ${DUKE_LIBRARY_DIRS}
       ${COMM_DEPS_LIBRARY_DIRS}
     ${WAYLAND_EGL_LIBRARY_DIRS}
  ${WAYLAND_CLIENT_LIBRARY_DIRS}
                ${PXSCENELIBDIR}
                          ${UWS}
)

set(PXSCENETEST_LINK_LIBRARIES ${PXSCENETEST_LINK_LIBRARIES}
               ${NODE_LIBRARIES}
               ${DUKE_LIBRARIES}
          ${COMM_DEPS_LIBRARIES}
        ${WAYLAND_EGL_LIBRARIES}
                             uWS
)

message(STATUS "NODE_LIBRARY_DIRS: ${NODE_LIBRARY_DIRS}")
message(STATUS "DUKE_LIBRARY_DIRS: ${DUKE_LIBRARY_DIRS}")

# LINUX # LINUX # LINUX # LINUX # LINUX # LINUX # LINUX # LINUX # LINUX # LINUX # LINUX # LINUX # LINUX # 

if (CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
    set(PXSCENETEST_LINK_DIRECTORIES ${PXSCENETEST_LINK_DIRECTORIES} ${PXCOREDIR}/remote)
    set(PXSCENETEST_LINK_LIBRARIES Spark pxCore rtCore_s ${PXSCENETEST_LINK_LIBRARIES} rt dl pthread)

    if (HOSTNAME STREQUAL "raspberrypi")

        message("Unit tests are not currently supported on raspberry pi")

    elseif (BUILD_WITH_WINDOWLESS_EGL)
        message("Building unit tests for egl")

        set(PX_PLATFORM PX_PLATFORM_GENERIC_EGL)
        set(PXSCENETEST_LINK_DIRECTORIES ${PXSCENETEST_LINK_DIRECTORIES} ${PXCOREDIR}/build/egl)
        set(PXSCENETEST_LINK_LIBRARIES ${PXSCENETEST_LINK_LIBRARIES} westeros_compositor)

        add_definitions(-DRT_PLATFORM_LINUX -DENABLE_NODE_V_6_9 -DNODE_PLATFORM="linux" -DNODE_WANT_INTERNALS=1 -DV8_DEPRECATION_WARNINGS=1 -DNODE_SHARED_MODE -DNODE_USE_V8_PLATFORM=1 -DNODE_HAVE_I18N_SUPPORT=1 -DNODE_HAVE_SMALL_ICU=1 -DHAVE_INSPECTOR=1 -DV8_INSPECTOR_USE_STL=1 -DV8_INSPECTOR_USE_OLD_STL=1)

        set(BUILD_UNIT_TEST 1)
        #set(PLATFORM_TEST_FILES test_pxWayland.cpp test_pxWaylandContainer.cpp)
    else ()
        message("Building unit tests for glut")

        set(PX_PLATFORM PX_PLATFORM_GLUT)
        set(PXSCENETEST_LINK_DIRECTORIES ${PXSCENETEST_LINK_DIRECTORIES} ${PXCOREDIR}/build/glut ${EXTDIR}/breakpad/src/client/linux)
        set(PXSCENETEST_LINK_LIBRARIES ${PXSCENETEST_LINK_LIBRARIES} glut)

        add_definitions(-DRT_PLATFORM_LINUX -DENABLE_NODE_V_6_9 -DNODE_PLATFORM="linux" -DNODE_WANT_INTERNALS=1 -DV8_DEPRECATION_WARNINGS=1 -DNODE_SHARED_MODE -DNODE_USE_V8_PLATFORM=1 -DNODE_HAVE_I18N_SUPPORT=1 -DNODE_HAVE_SMALL_ICU=1 -DHAVE_INSPECTOR=1 -DV8_INSPECTOR_USE_STL=1 -DV8_INSPECTOR_USE_OLD_STL=1)

        set(BUILD_UNIT_TEST 1)
        set(TEST_WAYLAND_SOURCE_FILES "")
        #set(TEST_WAYLAND_SOURCE_FILES test_pxWayland.cpp test_pxWaylandContainer.cpp)
        set(PLATFORM_TEST_FILES test_jsfiles.cpp test_pxUtil.cpp test_screenshot.cpp)

        if (PXSCENE_TEST_HTTP_CACHE)
                message("Enabling http cache tests")
                set(PLATFORM_TEST_FILES ${PLATFORM_TEST_FILES} test_imagecache.cpp)
        endif (PXSCENE_TEST_HTTP_CACHE)

        if (DEFINED ENV{CODE_COVERAGE})
            message("enabling code coverage support")
            add_definitions(-DENABLE_CODE_COVERAGE)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage -fno-inline -fno-elide-constructors")
            set(PLATFORM_LIBRARIES ${PLATFORM_LIBRARIES} gcov)
        endif () # CODE_COVERAGE

    endif (HOSTNAME STREQUAL "raspberrypi")

# APPLE # APPLE # APPLE # APPLE # APPLE # APPLE # APPLE # APPLE # APPLE # APPLE # APPLE # APPLE # APPLE # 
elseif (APPLE)
    message("Building unit tests for mac")

    include_directories(AFTER ${COMM_DEPS_INCLUDE_DIRS})

    set(PX_PLATFORM PX_PLATFORM_MAC)
    set(PXCORE_LIB_LOCATION ${PXCOREDIR}/build/mac)
    set(PXSCENETEST_LINK_DIRECTORIES ${PXSCENETEST_LINK_DIRECTORIES} ${PXCORE_LIB_LOCATION} ${COMM_DEPS_LIBRARY_DIRS})
    set(PLATFORM_LIBRARIES rtCore_s pxCore Spark dl ${COMM_DEPS_LIBRARIES} )
    set(PLATFORM_LIBRARIES ${PLATFORM_LIBRARIES} "-framework OpenGL -framework Cocoa -framework Foundation")

    add_definitions(${COMM_DEPS_DEFINITIONS} -DRT_PLATFORM_LINUX -DENABLE_NODE_V_6_9 -DNODE_PLATFORM="linux" -DNODE_WANT_INTERNALS=1 -DV8_DEPRECATION_WARNINGS=1 -DNODE_SHARED_MODE -DNODE_USE_V8_PLATFORM=1 -DNODE_HAVE_I18N_SUPPORT=1 -DNODE_HAVE_SMALL_ICU=1 -DHAVE_INSPECTOR=1 -DV8_INSPECTOR_USE_STL=1 -DV8_INSPECTOR_USE_OLD_STL=1)

    set(BUILD_UNIT_TEST 1)
    set(TEST_WAYLAND_SOURCE_FILES "")
# Some do not work on Mac ... Why ? >>    set(PLATFORM_TEST_FILES test_jsfiles.cpp test_pxUtil.cpp test_screenshot.cpp)
    set(PLATFORM_TEST_FILES "test_pxUtil.cpp")

    if (DEFINED ENV{CODE_COVERAGE})
        message("enabling code coverage support")
        add_definitions(-DENABLE_CODE_COVERAGE)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage -fno-inline -fno-elide-constructors")

        execute_process(COMMAND clang --version OUTPUT_VARIABLE LLVM_VERSION1 ERROR_VARIABLE ERROR_VARIABLE1 COMMAND grep "LLVM version" OUTPUT_VARIABLE LLVM_VERSION2 ERROR_VARIABLE LLVM_ERROR2 COMMAND awk "{print $4}" OUTPUT_FILE llvmoutput OUTPUT_VARIABLE LLVM_VERSION3 ERROR_VARIABLE LLVM_ERROR3)
        execute_process(COMMAND awk "{ printf \"%s\", $0 }" llvmoutput OUTPUT_VARIABLE LLVM_VERSION)
        execute_process(COMMAND rm llvmoutput)

        set(TEST_APP_LINKER_OPTIONS ${TEST_APP_LINKER_OPTIONS} "-L/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/lib/clang/${LLVM_VERSION}/lib/darwin -lclang_rt.profile_osx")
    endif () # CODE_COVERAGE

# NONE # NONE # NONE # NONE # NONE # NONE # NONE # NONE # NONE # NONE # NONE # NONE # NONE # NONE # NONE #
else()

    message("Unit tests are not currently supported on this platform")
endif (CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")


# COMMON # COMMON # COMMON # COMMON # COMMON # COMMON # COMMON # COMMON # COMMON # COMMON # COMMON # COMMON

add_definitions(-D${PX_PLATFORM} -DENABLE_RT_NODE -DRUNINMAIN -DENABLE_HTTP_CACHE)

set(TEST_SOURCE_FILES pxscene2dtestsmain.cpp  test_example.cpp test_api.cpp  test_pxcontext.cpp test_memoryleak.cpp test_rtnode.cpp test_rtMutex.cpp test_pxImage9Border.cpp test_eventListeners.cpp
    test_pxAnimate.cpp test_rtFile.cpp test_rtZip.cpp test_rtString.cpp test_rtValue.cpp test_pxImage.cpp test_pxOffscreen.cpp test_pxMatrix4T.cpp test_rtObject.cpp
    test_pxWindowUtil.cpp test_pxTexture.cpp test_pxWindow.cpp test_ioapi.cpp test_rtLog.cpp test_pxTimerNative.cpp
    test_rtUrlUtils.cpp test_pxArchive.cpp test_pxPixel_h.cpp test_pxFont.cpp test_rtThreadPool.cpp test_utf8.cpp
    test_rtSettings.cpp test_cors.cpp  test_external.cpp test_pxScene2d.cpp test_oscillate.cpp test_rtPathUtils.cpp
    test_rtError.cpp test_import_resources.cpp test_rtHttpRequest.cpp test_rtHttpResponse.cpp
    ${PLATFORM_TEST_FILES} ${TEST_WAYLAND_SOURCE_FILES})

if (DEFINED ENV{USE_HTTP_CACHE})
    message("Include http cache tests")
    set(TEST_SOURCE_FILES ${TEST_SOURCE_FILES} test_imagecache.cpp)
endif (DEFINED ENV{USE_HTTP_CACHE})

if (PXSCENE_TEST_PERMISSIONS_CHECK)
    message("Include PERMISSIONS tests")
    add_definitions(-DENABLE_PERMISSIONS_CHECK)
    set(TEST_SOURCE_FILES ${TEST_SOURCE_FILES} test_rtPermissions.cpp)
endif (PXSCENE_TEST_PERMISSIONS_CHECK)

set(TEST_SOURCE_FILES ${TEST_SOURCE_FILES} ${EXTDIR}/gtest/googletest/src/gtest-all.cc ${EXTDIR}/gtest/googlemock/src/gmock-all.cc)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -fpermissive -Wall -Wno-attributes -Wall -Wextra -Wno-format-security -Werror -std=c++11 -O3")

if (BUILD_UNIT_TEST GREATER 0)
    link_directories(${PXSCENETEST_LINK_DIRECTORIES})
    add_executable(pxscene2dtests ${TEST_SOURCE_FILES})
    target_link_libraries(pxscene2dtests ${PXSCENETEST_LINK_LIBRARIES} ${PLATFORM_LIBRARIES} ${TEST_APP_LINKER_OPTIONS})
endif (BUILD_UNIT_TEST GREATER 0)
