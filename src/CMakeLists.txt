cmake_minimum_required(VERSION 2.8)
project(pxcore)
find_package(PkgConfig)

set(CMAKE_CXX_STANDARD 11)

option(WINDOWLESS_EGL "WINDOWLESS_EGL" OFF)
option(PXCORE_WAYLAND_EGL "PXCORE_WAYLAND_EGL" OFF)
option(PXCORE_ESSOS "PXCORE_ESSOS" OFF)
option(PXCORE_MATRIX_HELPERS "PXCORE_MATRIX_HELPERS" ON)
option(PXCORE_PERMISSIONS_CHECK "PXCORE_PERMISSIONS_CHECK" ON)
option(BUILD_RTCORE_LIBS "BUILD_RTCORE_LIBS" ON)
option(BUILD_PXCORE_LIBS "BUILD_PXCORE_LIBS" ON)
option(OUTPUT_LIBS_LOCAL "OUTPUT_LIBS_LOCAL" OFF)
option(PXCORE_WAYLAND_DISPLAY_READ_EVENTS "PXCORE_WAYLAND_DISPLAY_READ_EVENTS" ON)
if(WIN32)
    option(PXCORE_COMPILE_WARNINGS_AS_ERRORS "PXCORE_COMPILE_WARNINGS_AS_ERRORS" OFF)
elseif (APPLE)
    option(PXCORE_COMPILE_WARNINGS_AS_ERRORS "PXCORE_COMPILE_WARNINGS_AS_ERRORS" ON)
else()
    option(PXCORE_COMPILE_WARNINGS_AS_ERRORS "PXCORE_COMPILE_WARNINGS_AS_ERRORS" ON)
endif(WIN32)

set(EXTDIR "${CMAKE_CURRENT_SOURCE_DIR}/../examples/pxScene2d/external")
set(NODEDIR "${EXTDIR}/node/")
include(../cmake/CommOpts.cmake)
include(../cmake/CommDeps.cmake)
include(../cmake/NodeDeps.cmake)
include(../cmake/V8Deps.cmake)

set(BUILD_RTCORE_SHARED_LIBRARY 1)

if (APPLE)
    message("Building pxcore for mac")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -g")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated -fPIC -g")
    set(CMAKE_MACOSX_RPATH 1)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../build/mac)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../build/mac)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../build/mac)
    add_definitions(-DPX_PLATFORM_MAC -DENABLE_RT_NODE -DENABLE_HTTP_CACHE -DRUNINMAIN)
    add_definitions(-D_DARWIN_USE_64_BIT_INODE=1 -DNODE_ARCH="x64" -DNODE_WANT_INTERNALS=1 -DV8_DEPRECATION_WARNINGS= -DNODE_SHARED_MODE
            -DNODE_USE_V8_PLATFORM=1 -DNODE_HAVE_I18N_SUPPORT=1 -DNODE_HAVE_SMALL_ICU=1 -DHAVE_INSPECTOR=1 -DV8_INSPECTOR_USE_STL=1
            -DV8_INSPECTOR_USE_OLD_STL=1 -DHAVE_OPENSSL=1 -DHAVE_DTRACE=1 -D__POSIX__ -DNODE_PLATFORM=darwin
            -DUCONFIG_NO_TRANSLITERATION=1 -DUCONFIG_NO_SERVICE=1 -DUCONFIG_NO_REGULAR_EXPRESSIONS=1 -DU_ENABLE_DYLOAD=0
            -DU_STATIC_IMPLEMENTATION=1 -DU_HAVE_STD_STRING=0 -DUCONFIG_NO_BREAK_ITERATION=0 -DUCONFIG_NO_LEGACY_CONVERSION=1
            -DUCONFIG_NO_CONVERSION=1 -DHTTP_PARSER_STRICT=0 -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64
            -DRT_PLATFORM_LINUX -DENABLE_NODE_V_6_9)
    set(PXCORE_FILES mac/pxBufferNative.mm mac/pxOffscreenNative.mm mac/pxWindowNative.mm
            mac/pxEventLoopNative.mm mac/pxTimerNative.mm mac/pxTimerNative.mm
            mac/pxClipboardNative.mm mac/window.mm
            unix/rtMutexNative.cpp unix/rtThreadPoolNative.cpp
            rtFileCache.cpp rtHttpCache.cpp)
    set(PLATFORM_LINKER_FLAGS "-framework Cocoa -framework Foundation")
    add_definitions(${COMM_DEPS_DEFINITIONS})
    include_directories(AFTER ${COMM_DEPS_INCLUDE_DIRS})
    if (DEFINED ENV{CODE_COVERAGE})
        message("enabling code coverage support")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage -fno-inline")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage -fno-inline")
        add_definitions(-DENABLE_CODE_COVERAGE=1)
    endif ()
elseif (CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -g")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated -fPIC -g")
    execute_process(COMMAND "hostname" OUTPUT_VARIABLE HOSTNAME)
    string(STRIP ${HOSTNAME} HOSTNAME)

    add_definitions(${COMM_DEPS_DEFINITIONS})

    include_directories(AFTER
               ${NODE_INCLUDE_DIRS} ${V8_INCLUDE_DIRS}
          ${COMM_DEPS_INCLUDE_DIRS}
    )

    if (HOSTNAME STREQUAL "raspberrypi")
        message("Building pxcore for raspberry pi")
        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../build/egl)
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../build/egl)
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../build/egl)
        add_definitions(-DENABLE_NODE_V_6_9 -DNODE_PLATFORM=linux -DNODE_WANT_INTERNALS=1 -DV8_DEPRECATION_WARNINGS=1 -DNODE_SHARED_MODE -DNODE_USE_V8_PLATFORM=1 -DNODE_HAVE_I18N_SUPPORT=1 -DNODE_HAVE_SMALL_ICU=1 -DHAVE_INSPECTOR=1 -DV8_INSPECTOR_USE_STL=1 -DV8_INSPECTOR_USE_OLD_STL=1)
        add_definitions(-DPX_PLATFORM_GENERIC_EGL -DRT_PLATFORM_LINUX -DENABLE_RT_NODE -DENABLE_HTTP_CACHE -DRUNINMAIN)
        set(PXCORE_FILES gles/pxBufferNative.cpp gles/pxOffscreenNative.cpp gles/pxWindowNative.cpp
                gles/pxEventLoopNative.cpp gles/pxTimerNative.cpp
                gles/pxClipboardNative.cpp gles/pxEGLProviderRPi.cpp gles/LinuxInputEventDispatcher.cpp pxViewWindow.cpp
                unix/rtMutexNative.cpp unix/rtThreadPoolNative.cpp
                rtFileCache.cpp rtHttpCache.cpp)
        set(PLATFORM_LINKER_FLAGS)
        include_directories(AFTER /opt/vc/include /opt/vc/include/interface/vcos/pthreads
                /opt/vc/include/interface/vmcs_host/linux /opt/vc/include)
        include_directories(AFTER ${ZLIBINC} ${CURLINC} ${JPGINC} ${JPGTURBOINC} ${PNGINC})
    elseif (WINDOWLESS_EGL OR PXCORE_WAYLAND_EGL OR PXCORE_ESSOS)
        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../build/egl)
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../build/egl)
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../build/egl)
        add_definitions(-DENABLE_NODE_V_6_9 -DNODE_PLATFORM=linux -DNODE_WANT_INTERNALS=1 -DV8_DEPRECATION_WARNINGS=1 -DNODE_SHARED_MODE -DNODE_USE_V8_PLATFORM=1 -DNODE_HAVE_I18N_SUPPORT=1 -DNODE_HAVE_SMALL_ICU=1 -DHAVE_INSPECTOR=1 -DV8_INSPECTOR_USE_STL=1 -DV8_INSPECTOR_USE_OLD_STL=1)
        add_definitions(-DRT_PLATFORM_LINUX -DPX_NO_WINDOW -DENABLE_RT_NODE -DENABLE_HTTP_CACHE -DRUNINMAIN -DRAPIDJSON_HAS_STDSTRING)
        set(PXCORE_FILES unix/rtMutexNative.cpp unix/rtThreadPoolNative.cpp
                rtFileCache.cpp rtHttpCache.cpp)

        if (PXCORE_ESSOS)
            message("Building pxcore for essos windows")
            add_definitions(-DPX_PLATFORM_ESSOS -DPX_PLATFORM_WAYLAND_EGL)
            set(PXCORE_FILES ${PXCORE_FILES} essos/pxWindowNative.cpp essos/pxBufferNative.cpp essos/pxOffscreenNative.cpp
                    essos/pxEventLoopNative.cpp essos/pxTimerNative.cpp essos/pxClipboardNative.cpp)
        elseif (PXCORE_WAYLAND_EGL)
            message("Building pxcore for wayland windows")
            add_definitions(-DPX_PLATFORM_WAYLAND_EGL)
            if (PXCORE_WAYLAND_DISPLAY_READ_EVENTS)
                message("Enabling wayland display read events")
                add_definitions(-DPXCORE_WL_DISPLAY_READ_EVENTS)
            endif(PXCORE_WAYLAND_DISPLAY_READ_EVENTS)
            set(PXCORE_FILES ${PXCORE_FILES} wayland_egl/pxWindowNative.cpp wayland_egl/pxBufferNative.cpp wayland_egl/pxOffscreenNative.cpp
                wayland_egl/pxEventLoopNative.cpp wayland_egl/pxTimerNative.cpp wayland_egl/pxClipboardNative.cpp)
        elseif (WINDOWLESS_EGL)
            message("Building pxcore for windowless egl")
            add_definitions(-DPX_PLATFORM_GENERIC_EGL)
            set(PXCORE_FILES ${PXCORE_FILES} gles/pxBufferNative.cpp gles/pxOffscreenNative.cpp
                gles/pxEventLoopNative.cpp gles/pxTimerNative.cpp gles/pxClipboardNative.cpp)
        endif (PXCORE_ESSOS)
        set(PLATFORM_LINKER_FLAGS)
    elseif (PXCORE_DFB)
        message("Building pxcore for dfb ")
        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../build/dfb)
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../build/dfb)
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../build/dfb)
    	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")
    	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
        set(PXCORE_INCLUDES $ENV{PXCORE_INCLUDES})
        separate_arguments(PXCORE_INCLUDES)
	include_directories(BEFORE ${PXCORE_INCLUDES})
        add_definitions(-DRT_PLATFORM_LINUX -DPX_NO_WINDOW -DENABLE_RT_NODE -DENABLE_HTTP_CACHE -DRUNINMAIN -DRAPIDJSON_HAS_STDSTRING -DENABLE_DFB -DENABLE_DFB_GENERIC -DBSTD_CPU_ENDIAN=BSTD_ENDIAN_LITTLE -DPX_PLATFORM_GENERIC_DFB)
        set(PXCORE_FILES x11/pxBufferNativeDfb.cpp x11/pxOffscreenNativeDfb.cpp x11/pxEventLoopNative.cpp x11/pxTimerNative.cpp x11/pxClipboardNative.cpp unix/rtMutexNative.cpp unix/rtThreadPoolNative.cpp rtFileCache.cpp rtHttpCache.cpp)
    else ()
        message("Building pxcore for glut")
        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../build/glut)
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../build/glut)
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../build/glut)
        add_definitions(-DENABLE_NODE_V_6_9 -DNODE_PLATFORM=linux -DNODE_WANT_INTERNALS=1 -DV8_DEPRECATION_WARNINGS=1 -DNODE_SHARED_MODE -DNODE_USE_V8_PLATFORM=1 -DNODE_HAVE_I18N_SUPPORT=1 -DNODE_HAVE_SMALL_ICU=1 -DHAVE_INSPECTOR=1 -DV8_INSPECTOR_USE_STL=1 -DV8_INSPECTOR_USE_OLD_STL=1)
        add_definitions(-DPX_PLATFORM_GLUT -DRT_PLATFORM_LINUX -DENABLE_RT_NODE -DENABLE_HTTP_CACHE -DRUNINMAIN)
        set(PLATFORM_LIBRARIES ${PLATFORM_LIBRARIES} glut)
        set(PXCORE_FILES glut/pxBufferNative.cpp glut/pxOffscreenNative.cpp glut/pxWindowNative.cpp
                glut/pxEventLoopNative.cpp glut/pxTimerNative.cpp glut/pxClipboardNative.cpp pxViewWindow.cpp
                unix/rtMutexNative.cpp unix/rtThreadPoolNative.cpp
                rtFileCache.cpp rtHttpCache.cpp)
        set(PLATFORM_LINKER_FLAGS)
        if (DEFINED ENV{CODE_COVERAGE})
            message("enabling code coverage support")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage -fno-inline")
            set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage -fno-inline")
            add_definitions(-DENABLE_CODE_COVERAGE=1)
        endif ()
    endif (HOSTNAME STREQUAL "raspberrypi")
elseif(WIN32)
    message("Building pxcore for Windows")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT /Zi /DEBUG")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../build/win)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../build/win)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../build/win)
    add_definitions(-DENABLE_NODE_V_6_9 -DNODE_PLATFORM=win32 -DNODE_WANT_INTERNALS=1 -DV8_DEPRECATION_WARNINGS=1 -DNODE_SHARED_MODE -DNODE_USE_V8_PLATFORM=1 -DNODE_HAVE_I18N_SUPPORT=1 -DNODE_HAVE_SMALL_ICU=1 -DHAVE_INSPECTOR=1 -DV8_INSPECTOR_USE_STL=1 -DV8_INSPECTOR_USE_OLD_STL=1)
    add_definitions(-DENABLE_RT_NODE -DRUNINMAIN)
    add_definitions(-DPX_PLATFORM_WIN -DRT_PLATFORM_WINDOWS -DWIN32 -DWIN32_LEAN_AND_MEAN -DGLEW_STATIC -D_TIMESPEC_DEFINED -D_CONSOLE
            -DCURL_STATICLIB -DRUNINMAIN -DENABLE_RT_NODE -DDISABLE_WAYLAND -DNODE_WANT_INTERNALS=1
            -DENABLE_NODE_V_6_9 -DENABLE_V8_HEAP_PARAMS=1 -DV8_DEPRECATION_WARNINGS=1 -DNODE_SHARED_MODE -DHAVE_INSPECTOR=1
            -DV8_INSPECTOR_USE_STL=1 -DV8_INSPECTOR_USE_OLD_STL=1 -DENABLE_MAX_TEXTURE_SIZE
            -DHAVE_OPENSSL -D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE -D_HAS_EXCEPTIONS=0 -DBUILDING_V8_SHARED=1
            -DBUILDING_UV_SHARED=1 -DNODE_ARCH="ia32" -DNODE_USE_V8_PLATFORM=1 -DNODE_HAVE_I18N_SUPPORT=1 -DNODE_HAVE_SMALL_ICU=1
            -DHAVE_OPENSSL=1 -DHAVE_ETW=1 -DHAVE_PERFCTR=1 -DFD_SETSIZE=1024 -DNODE_PLATFORM="win32" -D_UNICODE=1 -DUCONFIG_NO_TRANSLITERATION=1
            -DUCONFIG_NO_SERVICE=1 -DUCONFIG_NO_REGULAR_EXPRESSIONS=1 -DU_ENABLE_DYLOAD=0 -DU_STATIC_IMPLEMENTATION=1
            -DU_HAVE_STD_STRING=0 -DUCONFIG_NO_BREAK_ITERATION=0 -DUCONFIG_NO_LEGACY_CONVERSION=1 -DUCONFIG_NO_CONVERSION=1
            -DHTTP_PARSER_STRICT=0 -DUSE_RENDER_STATS -D_HAS_EXCEPTIONS=0)
    add_definitions(-DWIN32 -D_LIB -DNDEBUG -DPX_PLATFORM_WIN -DRT_PLATFORM_WINDOWS)
    add_definitions(${COMM_DEPS_DEFINITIONS})
    include_directories(AFTER ${COMM_DEPS_INCLUDE_DIRS})
    include_directories(AFTER "${EXTDIR}/pthread-2.9")
    set(PXCORE_FILES win/pxBufferNative.cpp win/pxOffscreenNative.cpp win/pxWindowNative.cpp
            win/pxEventLoopNative.cpp win/pxTimerNative.cpp win/pxClipboardNative.cpp pxViewWindow.cpp
            win/rtMutexNative.cpp win/rtThreadPoolNative.cpp)
    set_source_files_properties(utf8.c PROPERTIES LANGUAGE CXX)
    set(BUILD_RTCORE_SHARED_LIBRARY 0)
else ()
    message(FATAL_ERROR "Cannot build pxcore.  Unknown platform")
endif (APPLE)

if (NOT PXCORE_DFB)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive -std=c++11")
endif (NOT PXCORE_DFB)

if (PXCORE_COMPILE_WARNINGS_AS_ERRORS)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
endif (PXCORE_COMPILE_WARNINGS_AS_ERRORS)

include_directories(AFTER ${V8_INCLUDE_DIRS})
include_directories(AFTER ${NODE_INCLUDE_DIRS})
include_directories(AFTER ${CMAKE_CURRENT_SOURCE_DIR})

set(PXCORE_FILES ${PXCORE_FILES} pxOffscreen.cpp pxWindowUtil.cpp

        rtFile.cpp rtLibrary.cpp rtPathUtils.cpp rtTest.cpp rtThreadPool.cpp
        rtThreadQueue.cpp rtThreadTask.cpp rtUrlUtils.cpp
        rtZip.cpp pxInterpolators.cpp pxUtil.cpp
        rtFileDownloader.cpp unzip.c ioapi.c
        rtScript.cpp rtSettings.cpp rtCORS.cpp)
        
if (SUPPORT_DUKTAPE)
    message("Adding Duktape scripting support")
    add_definitions(-DRTSCRIPT_SUPPORT_DUKTAPE)
    set(PXCORE_FILES ${PXCORE_FILES} rtScriptDuk/rtScriptDuk.cpp rtScriptDuk/rtFunctionWrapperDuk.cpp rtScriptDuk/rtObjectWrapperDuk.cpp
        rtScriptDuk/rtWrapperUtilsDuk.cpp rtScriptDuk/rtJsModules.cpp)
endif()

if (SUPPORT_NODE)
    message("Adding Node scripting support")
    add_definitions(-DRTSCRIPT_SUPPORT_NODE)
    set(PXCORE_FILES ${PXCORE_FILES} rtScriptV8/rtScriptNode.cpp rtScriptV8/jsCallback.cpp rtScriptV8/rtFunctionWrapper.cpp 
        rtScriptV8/rtObjectWrapper.cpp rtScriptV8/rtWrapperUtils.cpp)
endif()

if (SUPPORT_V8)
    message("Adding V8 scripting support")
    add_definitions(-DRTSCRIPT_SUPPORT_V8)
    set(PXCORE_FILES ${PXCORE_FILES} rtScriptV8/rtScriptV8.cpp rtScriptV8/jsCallback.cpp rtScriptV8/rtFunctionWrapper.cpp 
        rtScriptV8/rtObjectWrapper.cpp rtScriptV8/rtWrapperUtils.cpp ${V8_SOURCES})
endif()


if (PXCORE_MATRIX_HELPERS)
  set(PXCORE_FILES ${PXCORE_FILES} pxMatrix4T.cpp)
endif(PXCORE_MATRIX_HELPERS)

if (PXCORE_PERMISSIONS_CHECK)
    add_definitions(-DENABLE_PERMISSIONS_CHECK)
    set(PXCORE_FILES ${PXCORE_FILES} rtPermissions.cpp)
endif (PXCORE_PERMISSIONS_CHECK)

set(RTCORE_FILES utf8.c rtString.cpp rtLog.cpp rtValue.cpp rtError.cpp ioapi_mem.c)

set(RTCORE_FILES ${RTCORE_FILES} rtPromise.cpp)

set(RTCORE_FILES ${RTCORE_FILES} rtObject.cpp)

include_directories(AFTER ${EXTDIR}/nanosvg/src)

if (OUTPUT_LIBS_LOCAL)
    message("Output pxCore and rtCore libs locally")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif (OUTPUT_LIBS_LOCAL)

#if (USE_DUKTAPE)
    include_directories(AFTER ${EXTDIR}/dukluv/lib/duktape/src)
    include_directories(AFTER ${EXTDIR}/dukluv/lib/uv/include)
    include_directories(AFTER ${EXTDIR}/dukluv/src)
#endif(USE_DUKTAPE)

if (BUILD_PXCORE_LIBS)
    message("Building pxcore libs")
    add_library(pxCore ${PXCORE_FILES})
endif (BUILD_PXCORE_LIBS)
if (BUILD_RTCORE_LIBS)
    message("Building rtcore libs")
    add_library(rtCore_s ${RTCORE_FILES})
    if (BUILD_RTCORE_SHARED_LIBRARY GREATER 0)
      add_library(rtCore SHARED ${RTCORE_FILES})
    endif(BUILD_RTCORE_SHARED_LIBRARY GREATER 0)
endif (BUILD_RTCORE_LIBS)
